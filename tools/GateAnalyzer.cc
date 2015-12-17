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

GateAnalyzer::GateAnalyzer(ClauseList* clauseList) {
  clauses = clauseList;
  index = new ClauseIndex(clauses);

  parents = new map<Literal, vector<Literal>*>();
  inputs = new map<Literal, bool>();
  projection = new Projection();

  minisat = NULL;

  // introduce new variable and unit-clause
  root = mkLit(clauses->newVar(), false);
  PooledLiterals* rootClause = new PooledLiterals(root);
  rootClause->setMarked();

  // create some data-structures for each variable
  gates = new vector<Gate*>();
  for (int i = 0; i <= clauses->maxVar(); i++) {
    (*parents)[mkLit(i, false)] = new vector<Literal>();
    (*parents)[mkLit(i, true)] = new vector<Literal>();
    (*inputs)[mkLit(i, false)] = false;
    (*inputs)[mkLit(i, true)] = false;
    gates->push_back(NULL);
  }

  // create artificial root unit-clause and subordinate the existing unit-clauses
  ClauseList* units = clauses->getByCriteria(createUnitFilter());
  clauses->add(rootClause); // note: it is important to add the new unit _after_ filtering the existing units
  index->add(rootClause);
  for (unsigned int i = 0; i < units->size(); i++) {
    index->augment(units->get(i), ~root);
  }
}

GateAnalyzer::~GateAnalyzer() {
  for (map<Literal, vector<Literal>*>::iterator it = parents->begin();
      it != parents->end(); ++it) {
    delete it->second;
  }
  delete index;
  delete parents;
}

MinisatSolver* GateAnalyzer::getMinisatSolver() {
  if (minisat == NULL) {
    minisat = new MinisatSolver(clauses);
  }
  return minisat;
}

ClauseList* GateAnalyzer::getGateClauses(Literal literal) {
  Gate* gate = (*gates)[var(literal)];
  if (gate == NULL || gate->getOutput() != literal)
    return NULL;
  return gate->getForwardClauses();
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
  if (gate != NULL) {
    return gate->getInputs();
  } else {
    return NULL;
  }
}

int GateAnalyzer::countInputs(Literal parent) {
  vector<Literal>* inputs = getInputs(parent);
  return inputs == NULL ? 0 : inputs->size();
}

bool GateAnalyzer::hasInputs(Literal parent) {
  return countInputs(parent) > 0;
}

vector<Literal>* GateAnalyzer::getParents(Literal child) {
  return (*parents)[child];
}

int GateAnalyzer::countParents(Literal child) {
  return getParents(child)->size();
}

bool GateAnalyzer::hasParents(Literal child) {
  return countParents(child) > 0;
}

void GateAnalyzer::setParent(Literal parent, Literal child) {
  D2(fprintf(stderr, "Adding Parent-child-relationship: %s%i -> %s%i\n", sign(parent)?"-":"", var(parent), sign(child)?"-":"", var(child));)
  if (std::find((*parents)[child]->begin(), (*parents)[child]->end(), parent) == (*parents)[child]->end()) {
    (*parents)[child]->push_back(parent);
  }
}

void GateAnalyzer::unsetParent(Literal parent) {
  vector<Literal>* inputs = getInputs(parent);
  for (vector<Literal>::iterator it = inputs->begin(); it != inputs->end();
      it++) {
    Literal child = *it;
    vector<Literal>* outputs = (*parents)[child];
    outputs->erase(std::remove(outputs->begin(), outputs->end(), parent),
        outputs->end());
  }
}

/**
 * Globally remember input polarities for (fast) monotonicity detection
 */
void GateAnalyzer::setAsInput(Literal literal) {
  (*inputs)[literal] = true;
}
bool GateAnalyzer::isUsedAsInput(Literal literal) {
  return (*inputs)[literal];
}
bool GateAnalyzer::isMonotonousInput(Var var) {
  return !isUsedAsInput(mkLit(var, false)) || !isUsedAsInput(mkLit(var, true));
}

/***
 * Algorithm Refinement, after submission to SAT:
 * 1. Conceptually add sth. like pure-decomposition for a better clause-selection heuristic
 *    by always picking the bunch of clauses for the selected literal (based on min-occurence heuristic).
 *    Maybe this make several tries more effective.
 * 2. Still need to implement the optimization of data-structure developed for the paper [O(1) monotonicity-check]
 */
ClauseList* GateAnalyzer::getNextClauses(ClauseList* list) {
  Literal min;
  int minOcc = INT_MAX;
  ClauseIndex* index = new ClauseIndex(list);
  for (int i = 0; i < list->maxVar(); i++) {
    Literal lit = mkLit(i, false);
    int occ = index->countOccurence(lit);
    if (occ != 0 && occ < minOcc) {
      minOcc = occ;
      min = lit;
    }
    occ = index->countOccurence(~lit);
    if (occ != 0 && occ < minOcc) {
      minOcc = occ;
      min = ~lit;
    }
  }
  return index->getClauses(min);
}

PooledLiterals* GateAnalyzer::getNextClause(ClauseList* list, RootSelectionMethod method) {
  switch (method) {
  case FIRST_CLAUSE: {
    return list->getFirst();
  }
  case MAX_ID: {
    Literal maxLit = mkLit(0, false);
    PooledLiterals* result = NULL;
    for (ClauseList::iterator clause = list->begin(); clause != list->end();
        clause++) {
      for (PooledLiterals::iterator clit = (*clause)->begin();
          clit != (*clause)->end(); clit++) {
        if (var(*clit) > var(maxLit)) {
          maxLit = *clit;
          result = *clause;
        }
      }
    }
    return result;
  }
  case MIN_OCCURENCE: {
    return getNextClauses(list)->getFirst();
  }
  default:
    return list->getLast();
  }
}

/***
 * The more general approach.
 * Analyze until all clauses are part of the hierarchy.
 * Select Clauses by the following heuristic:
 * 1. first select all unit-clauses
 * 2. if there is still a side-problem and no unit-clause left, select the one containing the biggest variable
 * 3. in the end forge together all selected root-clauses by one unit that implies them all
 */
void GateAnalyzer::analyzeEncoding(RootSelectionMethod selection, EquivalenceDetectionMethod equivalence, int tries) {
  analyzeEncoding(root, equivalence);

  ClauseList* remainder = clauses->getByCriteria(createNoMarkFilter());
  for (int count = 0; count < tries && remainder->size() > 0; count++) {
    PooledLiterals* next = getNextClause(remainder, selection);

    next->setMarked();
    getOrCreateGate(root)->addForwardClause(next);
    for (PooledLiterals::iterator it = next->begin(); it != next->end(); it++) {
      setParent(root, *it);
    }
    index->augment(next, ~root);

    for (PooledLiterals::iterator it = next->begin(); it != next->end(); it++) {
      if (*it != ~root)
        analyzeEncoding(*it, equivalence);
    }
    remainder->dumpByCriteria(createMarkFilter());
  }

  for (ClauseList::iterator it = remainder->begin(); it != remainder->end(); it++) {
    PooledLiterals* next = *it;
    next->setMarked();
    index->augment(next, ~root);
    getOrCreateGate(root)->addForwardClause(next);
    for (PooledLiterals::iterator it2 = next->begin(); it2 != next->end(); it2++) {
      setParent(root, *it2);
    }
  }

  delete remainder;
}

/*******
 * Create a DAG starting from a fact. Detect cycles and stop if necessary
 ***/
void GateAnalyzer::analyzeEncoding(Literal root, EquivalenceDetectionMethod equivalence) {
  D1(fprintf(stderr, "Root is %s%i\n", sign(root) ? "-" : "", var(root));)

  // a queue of literals that grows while checking for implicates
  vector<Literal>* literals = new vector<Literal>();
  literals->push_back(root);

  // while there are literals in the queue we check for children and add them to the queue
  for (unsigned int i_lit = 0; i_lit < literals->size(); i_lit++) {
    Literal output = (*literals)[i_lit];

    // Stop on projection (forced inputs)
    if (projectionContains(var(output))) {
      continue;
    }

    Gate* gate = NULL;

    ClauseList* fwd = index->getClauses(~output)->getByCriteria(createNoMarkFilter());
    ClauseList* bwd = index->getClauses(output)->getByCriteria(createNoMarkFilter());

    D1(
        fprintf(stderr, "Running Gate-Detection on %s%i\n", sign(output)?"-":"", var(output));
        //fwd->print(stderr);
        //bwd->print(stderr);
        //fprintf(stderr, "\n");
    )

    if (fwd->size() > 0 && bwd->isBlockedBy(output, fwd)) {
      if (isMonotonousInput(var(output))) {
        D1(fprintf(stderr, "Monotonous Parents %s%i\n", sign(output)?"-":"", var(output));)
        gate = defGate(output, fwd, bwd);
      } else if (isFullEncoding(output, fwd, bwd, equivalence)){ // non-monotonous
        D1(fprintf(stderr, "Non-Monotonous Parents %s%i\n", sign(output)?"-":"", var(output));)
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

bool GateAnalyzer::isFullEncoding(Literal output, ClauseList* fwd,
    ClauseList* bwd, EquivalenceDetectionMethod equivalence) {
  switch (equivalence) {
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

void GateAnalyzer::undefGate(Gate* gate) {
  gate->getForwardClauses()->unmarkAll();
  gate->getBackwardClauses()->unmarkAll();
  delete gate;
  (*gates)[var(gate->getOutput())] = NULL;
  unsetParent(gate->getOutput());
}

bool GateAnalyzer::isLitMonotonousInput(Literal output) {
  if (hasParents(~output)) {
    return false;
  }

  vector<Literal>* parents = getParents(output);
  for (vector<Literal>::iterator it = parents->begin(); it != parents->end();
      it++) {
    if (getGate(*it)->hasNonMonotonousParent()) {
      return false;
    }
  }

  return true;
}

void GateAnalyzer::setProjection(Projection* projection) {
  this->projection = projection;
}

bool GateAnalyzer::projectionContains(Var var) {
  if (this->projection == NULL || this->projection->size() == 0) {
    return false;
  } else {
    return this->projection->contains(var + 1);
  }
}

}
