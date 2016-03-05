/*
 * SolverMetaInfo.cc
 *
 *  Created on: 04.05.2013
 *      Author: markus
 */

#include <climits>
#include <stdio.h>
#include <stdlib.h>
#include <deque>

#include "../types/Literal.h"
#include "../types/ClauseList.h"
#include "../types/ClauseIndex.h"

#include "../filters/ClauseFilters.h"

#include "../minisat/MinisatSolver.h"

#include "Gate.h"
#include "Projection.h"
#include "GateAnalyzer.h"

#include <assert.h>

#include "../types/PooledLiterals.h"
#include "../types/FixedSizeLiterals.h"

#define VERBOSITY 0
#include "../misc/debug.h"

namespace Dark {

GateAnalyzer::GateAnalyzer(ClauseList* clauseList, RootSelectionMethod sel = MIN_OCCURENCE, EquivalenceDetectionMethod eq = PATTERNS) {
  clauses = clauseList;
  root = createRoot(clauseList);
  index = new ClauseIndex(clauses);
  selectionMethod = sel;
  equivalenceDetectionMethod = eq;

  parents = new map<Literal, vector<Literal>*>();
  inputs = new map<Literal, bool>();

  minisat = NULL;

  // create some data-structures for each variable
  gates = new vector<Gate*>(clauses->maxVar() + 1, NULL);
  for (int i = 0; i <= clauses->maxVar(); i++) {
    (*parents)[mkLit(i, false)] = new vector<Literal>();
    (*parents)[mkLit(i, true)] = new vector<Literal>();
    (*inputs)[mkLit(i, false)] = false;
    (*inputs)[mkLit(i, true)] = false;
  }
}

GateAnalyzer::~GateAnalyzer() {
  for (map<Literal, vector<Literal>*>::iterator it = parents->begin(); it != parents->end(); ++it) {
    delete it->second;
  }
  delete parents;
  delete inputs;
  delete index;
  for (Gate* gate : *gates) {
    if (gate != nullptr) {
      delete gate;
    }
  }
  delete gates;
}

Literal GateAnalyzer::createRoot(ClauseList* clauses) {
  Literal root = mkLit(clauses->newVar(), false);
  for (PooledLiterals* clause : *clauses) {
    if (clause->size() == 1) {
      clause->add(~root);
    }
  }
  // note: it is important to add the new unit _after_ filtering the existing units:
  PooledLiterals* rootClause = new PooledLiterals(root);
  clauses->add(rootClause);
  rootClause->setMarked();
  return root;
}

MinisatSolver* GateAnalyzer::getMinisatSolver() {
  if (minisat == NULL) {
    minisat = new MinisatSolver(clauses);
  }
  return minisat;
}

Literal GateAnalyzer::getRoot() {
  return root;
}

Gate* GateAnalyzer::getGate(Literal output) {
  Gate* gate = (*gates)[var(output)];
  if (gate != NULL && gate->getOutput() == output) {
    return gate;
  } else {
    return NULL;
  }
}

Gate* GateAnalyzer::getOrCreateGate(Literal output) {
  Gate* gate = (*gates)[var(output)];
  if (gate != NULL && gate->getOutput() == output) {
    return gate;
  } else {
    assert(gate == NULL && "gate must not exist");
    gate = new Gate(output, new ClauseList(), new ClauseList());
    (*gates)[var(output)] = gate;
    return gate;
  }
}

vector<Literal>* GateAnalyzer::getInputs(Literal parent) {
  Gate* gate = getGate(parent);
  return gate != NULL ? gate->getInputs() : NULL;
}

bool GateAnalyzer::hasInputs(Literal parent) {
  vector<Literal>* inputs = getInputs(parent);
  return inputs != NULL && inputs->size() > 0;
}

vector<Literal>* GateAnalyzer::getParents(Literal child) {
  return (*parents)[child];
}

void GateAnalyzer::setParent(Literal parent, Literal child) {
  D2(fprintf(stderr, "Adding Parent-child-relationship: %s%i -> %s%i\n", sign(parent)?"-":"", var(parent), sign(child)?"-":"", var(child));)
  if (std::find((*parents)[child]->begin(), (*parents)[child]->end(), parent) == (*parents)[child]->end()) {
    (*parents)[child]->push_back(parent);
  }
}

void GateAnalyzer::setAsInput(Literal literal) {
  (*inputs)[literal] = true;
}

bool GateAnalyzer::isMonotonousInput(Var var) {
  return !(*inputs)[mkLit(var, false)] || !(*inputs)[mkLit(var, true)];
}


/***
 * The more general approach.
 * Analyze until all clauses are part of the hierarchy.
 * Select Clauses by the following heuristic:
 * 1. first select all unit-clauses
 * 2. if there is still a side-problem and no unit-clause left, select set of of clauses by heuristic
 * 3. in the end forge together all selected root-clauses by one unit that implies them all
 */
void GateAnalyzer::analyzeEncoding(int tries) {
  analyzeEncodingForRoot(root);

  ClauseList* remainder = clauses->getByCriteria(createNoMarkFilter());
  for (int count = 0; count < tries && remainder->size() > 0; count++) {
    ClauseList* nextClauses = getNextClauses(remainder);

    // thread next clauses
    for (ClauseList::iterator clit = nextClauses->begin(); clit != nextClauses->end(); clit++) {
      PooledLiterals* clause = *clit;
      clause->setMarked();
      getOrCreateGate(root)->addForwardClause(clause);
      for (PooledLiterals::iterator it = clause->begin(); it != clause->end(); it++) {
        setParent(root, *it);
      }
      index->augment(clause, ~root);
    }

    // iterate new inputs
    for (ClauseList::iterator clit = nextClauses->begin(); clit != nextClauses->end(); clit++) {
      PooledLiterals* clause = *clit;
      for (PooledLiterals::iterator it = clause->begin(); it != clause->end(); it++) {
        if (*it != ~root)
          analyzeEncodingForRoot(*it);
      }
    }
    remainder->dumpByCriteria(createMarkFilter());
  }

  for (ClauseList::iterator it = remainder->begin(); it != remainder->end(); it++) {
    PooledLiterals* next = *it;
    next->setMarked();
    index->augment(next, ~root);
    getOrCreateGate(root)->addForwardClause(next);
    for (PooledLiterals::iterator it2 = next->begin(); *it2 != litFalse; it2++) {
      setParent(root, *it2);
    }
  }

  delete remainder;
}

/*******
 * Create a DAG starting from a fact. Detect cycles and stop if necessary
 ***/
void GateAnalyzer::analyzeEncodingForRoot(Literal root) {
  D1(fprintf(stderr, "Root is %s%i\n", sign(root) ? "-" : "", var(root));)

  // a queue of literals that grows while checking for implicates
  vector<Literal>* literals = new vector<Literal>();
  literals->push_back(root);

  // while there are literals in the queue we check for children and add them to the queue
  for (unsigned int i_lit = 0; i_lit < literals->size(); i_lit++) {
    Literal output = (*literals)[i_lit];

    Gate* gate = NULL;

    ClauseList* fwd = index->getClauses(~output)->getByCriteria(createNoMarkFilter());
    ClauseList* bwd = index->getClauses(output)->getByCriteria(createNoMarkFilter());

    D1(
        fprintf(stderr, "Running Gate-Detection on %s%i\n", sign(output)?"-":"", var(output));
        fwd->print(stderr);
        bwd->print(stderr);
        fprintf(stderr, "\n");
    )

    if (fwd->size() > 0 && bwd->isBlockedBy(output, fwd)) {
      if (isMonotonousInput(var(output))) {
        D1(fprintf(stderr, "Monotonous Parents %s%i\n", sign(output)?"-":"", var(output));)
        gate = defGate(output, fwd, bwd);
      } else if (isFullEncoding(output, fwd, bwd)) { // non-monotonous
        D1(fprintf(stderr, "Non-Monotonous Parents %s%i\n", sign(output)?"-":"", var(output)+1);)
        gate = defGate(output, fwd, bwd);
        gate->setHasNonMonotonousParent();
      }
    }

    if (gate != NULL) {
      literals->insert(literals->end(), gate->getInputs()->begin(), gate->getInputs()->end());
    } else {
      delete bwd;
      delete fwd;
    }
  }

  delete literals;
}

Gate* GateAnalyzer::defGate(Literal output, ClauseList* fwd, ClauseList* bwd) {
  fwd->markAll();
  bwd->markAll();
  Gate* gate = new Gate(output, fwd, bwd);
  (*gates)[var(output)] = gate;

  vector<Literal>* inputs = gate->getInputs();
  for (std::vector<Literal>::iterator lit = inputs->begin();
      lit != inputs->end(); ++lit) {
    setParent(output, *lit);
    setAsInput(*lit);
    if (!isMonotonousInput(var(output))) {
      setAsInput(~(*lit));
    }
  }

  return gate;
}

ClauseList* GateAnalyzer::getNextClauses(ClauseList* list) {
  ClauseIndex index {list};
  switch (selectionMethod) {
  case MIN_OCCURENCE: {
    Literal min;
    int minOcc = INT_MAX;
    for (int i = 0; i < list->maxVar(); i++) {
      Literal lit = mkLit(i, false);
      int occ = index.countOccurence(lit);
      if (occ != 0 && occ < minOcc) {
        minOcc = occ;
        min = lit;
      }
      occ = index.countOccurence(~lit);
      if (occ != 0 && occ < minOcc) {
        minOcc = occ;
        min = ~lit;
      }
    }
    ClauseList* result = new ClauseList();
    result->addAll(index.getClauses(min));
    return result;
  }
  case PURITY: {
    Var maxVar;
    double maxPurity = 0;
    for (int i = 0; i < list->maxVar(); i++) {
      Literal lit = mkLit(i, false);
      double pos = index.countOccurence(lit);
      double neg = index.countOccurence(~lit);
      double purity = abs(pos - neg) / (pos + neg);
      if (purity > maxPurity) {
        maxPurity = purity;
        maxVar = i;
      }
    }
    double pos = index.countOccurence(mkLit(maxVar, false));
    double neg = index.countOccurence(mkLit(maxVar, true));

    ClauseList* result = new ClauseList();
    result->addAll(pos < neg ? index.getClauses(mkLit(maxVar, false)) : index.getClauses(mkLit(maxVar, true)));
    return result;
  }
  default:
    return list;
  }
}

bool GateAnalyzer::isFullEncoding(Literal output, ClauseList* fwd, ClauseList* bwd) {
  switch (equivalenceDetectionMethod) {
  case PATTERNS:
    return bwd->matchesFullGatePattern(output, fwd);
  case SEMANTIC:
    if (bwd->matchesFullGatePattern(output, fwd)) {
      return true;
    }
    if (fwd->size() > 3 || fwd->maxClauseSize() > 4) {
      return false;
    }
    return semanticCheck(output, fwd);
  case SKIP:
  default:
    return false;
  }
}

bool GateAnalyzer::semanticCheck(Literal output, ClauseList* fwd) {
  int i = 0;
  vector<PooledLiterals*> cubes(fwd->size() + 1);
  cubes[0] = new PooledLiterals(~output);
  for (PooledLiterals* clause : *fwd) {
    cubes[++i] = clause->allBut(~output);
  }

  i = 0;
  vector<int> positions(cubes.size(), 0);
  vector<int> maxima(cubes.size());
  for (PooledLiterals* cube : cubes) {
    maxima[i++] = cube->size();
  }

  PooledLiterals* assumptions = new PooledLiterals();
  do {
    i = 0;
    for (PooledLiterals* cube : cubes) {
      assumptions->add(cube->get(positions[i++]));
    }
    if (getMinisatSolver()->isUPConsistent(assumptions)) {
      return false;
    }
  } while (increment(positions, maxima));

  return true;
}

bool GateAnalyzer::increment(vector<int>& digits, const vector<int> maxima) {
  unsigned int pos = 0;
  int newDigit;
  do {
    newDigit = (digits[pos] + 1) % maxima[pos];
  } while (newDigit == digits[pos++] && pos < digits.size());
  return pos < digits.size();
}

}
