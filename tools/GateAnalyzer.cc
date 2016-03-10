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

GateAnalyzer::GateAnalyzer(ClauseList* clauses, RootSelectionMethod sel, EquivalenceDetectionMethod eq) {
  this->clauses = clauses;

  selectionMethod = sel;
  equivalenceDetectionMethod = eq;

  gates = new vector<Gate*>(clauses->maxVar() + 1, NULL);
  inputs = new vector<bool>(2*clauses->maxVar() + 2, false);

  minisat = NULL;
}

GateAnalyzer::~GateAnalyzer() {
  delete inputs;
  for (Gate* gate : *gates) {
    if (gate != nullptr) {
      delete gate;
    }
  }
  delete gates;
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

vector<Literal>* GateAnalyzer::getInputs(Literal parent) {
  Gate* gate = getGate(parent);
  return gate != NULL ? gate->getInputs() : NULL;
}

bool GateAnalyzer::hasInputs(Literal parent) {
  vector<Literal>* inputs = getInputs(parent);
  return inputs != NULL && inputs->size() > 0;
}

void GateAnalyzer::setAsInput(Literal literal) {
  (*inputs)[toInt(literal)] = true;
}

bool GateAnalyzer::isMonotonousInput(Var var) {
  return !(*inputs)[2*var] || !(*inputs)[2*var+1];
}

void GateAnalyzer::createRootGate(ClauseList* roots) {
  root = mkLit(clauses->newVar(), false);
  clauses->add(new PooledLiterals(root));
  for (ClauseList::iterator clit = roots->begin(); clit != roots->end(); clit++) {
    (*clit)->add(~root);
  }
  (*gates)[var(root)] = new Gate(root, roots, NULL);
}

void GateAnalyzer::analyzeEncoding(int tries) {
  ClauseIndex* index = new ClauseIndex(clauses);

  ClauseList* roots = new ClauseList();
  ClauseList* next = new ClauseList();

  for (PooledLiterals* clause : *clauses) {
    if (clause->size() == 1) {
      next->add(clause);
    }
  }

  for (int count = 0; count < tries && next->size() > 0; count++) {
    roots->addAll(next);
    index->removeAll(next);

    for (ClauseList::iterator clit = next->begin(); clit != next->end(); clit++) {
      (*clit)->setMarked();
      for (PooledLiterals::iterator lit = (*clit)->begin(); *lit != litFalse; lit++) {
        setAsInput(*lit);
      }
    }

    // iterate new inputs
    for (ClauseList::iterator clit = next->begin(); clit != next->end(); clit++) {
      for (PooledLiterals::iterator lit = (*clit)->begin(); *lit != litFalse; lit++) {
        analyzeEncodingBFS(*lit, index);
      }
    }

    delete next;
    next = getNextClauses(index);
  }

  for (ClauseList::iterator clit = clauses->begin(); clit != clauses->end(); clit++) {
    if (!(*clit)->isMarked()) roots->add(*clit);
  }
  createRootGate(roots);
}

void GateAnalyzer::analyzeEncodingBFS(Literal root, ClauseIndex* index) {
  vector<Literal>* literals = new vector<Literal>();
  literals->push_back(root);

  // while there are literals in the queue we check for children and add them to the queue
  for (unsigned int i_lit = 0; i_lit < literals->size(); i_lit++) {
    Literal output = (*literals)[i_lit];

    ClauseList* fwd = index->getClauses(~output);
    ClauseList* bwd = index->getClauses(output);

    if (fwd->size() > 0 && bwd->isBlockedBy(output, fwd)) {
      bool monotonous = isMonotonousInput(var(output));
      if (monotonous || isFullEncoding(output, fwd, bwd)) {
        Gate* gate = defGate(output, fwd, bwd, monotonous);
        index->removeAll(gate->getForwardClauses());
        index->removeAll(gate->getBackwardClauses());
        literals->insert(literals->end(), gate->getInputs()->begin(), gate->getInputs()->end());
      }
    }
  }

  delete literals;
}

Gate* GateAnalyzer::defGate(Literal output, ClauseList* fwd, ClauseList* bwd, bool monotonous) {
  Gate* gate = new Gate(output, fwd, bwd);
  (*gates)[var(output)] = gate;
  fwd->markAll();
  bwd->markAll();

  if (!monotonous) {
    gate->setHasNonMonotonousParent();
  }

  vector<Literal>* inputs = gate->getInputs();
  for (std::vector<Literal>::iterator lit = inputs->begin(); lit != inputs->end(); ++lit) {
    setAsInput(*lit);
    if (!monotonous) {
      setAsInput(~(*lit));
    }
  }

  return gate;
}

ClauseList* GateAnalyzer::getNextClauses(ClauseIndex* index) {
  ClauseList* result = new ClauseList();
  switch (selectionMethod) {
  case MIN_OCCURENCE: {
    Literal min;
    int minOcc = INT_MAX;
    for (int i = 0; i < clauses->maxVar(); i++) {
      for (int b = 0; b < 2; b++) {
        Literal lit = mkLit(i, b % 2 == 0);
        int occ = index->countOccurence(lit);
        if (occ != 0 && occ < minOcc) {
          minOcc = occ;
          min = lit;
        }
      }
    }
    result->addAll(index->getClauses(min));
    break;
  }
  case PURITY: {
    Var maxVar;
    double maxPurity = 0;
    for (int i = 0; i < clauses->maxVar(); i++) {
      Literal lit = mkLit(i, false);
      double pos = index->countOccurence(lit);
      double neg = index->countOccurence(~lit);
      double purity = abs(pos - neg) / (pos + neg);
      if (purity > maxPurity) {
        maxPurity = purity;
        maxVar = i;
      }
    }
    double pos = index->countOccurence(mkLit(maxVar, false));
    double neg = index->countOccurence(mkLit(maxVar, true));

    result->addAll(pos < neg ? index->getClauses(mkLit(maxVar, false)) : index->getClauses(mkLit(maxVar, true)));
    break;
  }
  }
  return result;
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
