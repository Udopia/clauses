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
#include "../types/Literals.h"
#include "../types/MappedClauseList.h"

#include "../filters/ClauseFilters.h"

#include "../minisat/MinisatSolver.h"

#include "Gate.h"
#include "Projection.h"
#include "GateAnalyzer.h"

#include <assert.h>
#include "../types/FixedSizeLiterals.h"

#define VERBOSITY 0
#include "../debug.h"

namespace Dark {

GateAnalyzer::GateAnalyzer(ClauseList* clauseList, int full_eq_detection,
    bool use_refinement) {
  clauses = new MappedClauseList();
  clauses->addAll(clauseList);

  if (full_eq_detection > 1) {
    minisat = new MinisatSolver(clauses);
  } else {
    minisat = NULL;
  }

  parents = new map<Literal, vector<Literal>*>();
  projection = new Projection();

  this->use_refinement = use_refinement;
  this->full_eq_detection = full_eq_detection;

  gates = new vector<Gate*>();

  for (int i = 0; i < clauses->nVars(); i++) {
    newVar();
  }
}

GateAnalyzer::~GateAnalyzer() {
  freeAllContent();
  delete clauses;
  delete parents;
}

void GateAnalyzer::freeAllContent() {
  clauses->freeClauses();
  for (map<Literal, vector<Literal>*>::iterator it = parents->begin();
      it != parents->end(); ++it) {
    delete it->second;
  }
}

ClauseList* GateAnalyzer::getGateClauses(Literal literal) {
  Gate* gate = (*gates)[var(literal)];
  if (gate == NULL || gate->getOutput() != literal)
    return NULL;
  return gate->getForwardClauses();
}

int GateAnalyzer::maxVar() {
  D1(fprintf(stderr, "%i\n", max_var);)
  return max_var;
}

int GateAnalyzer::newVar() {
  max_var++;
  (*parents)[mkLit(max_var, false)] = new vector<Literal>();
  (*parents)[mkLit(max_var, true)] = new vector<Literal>();
  gates->push_back(NULL);
  return max_var;
}

ClauseList* GateAnalyzer::getRoots() {
  return clauses->getByCriteria(createUnitFilter());
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
    gate = new Gate(output, new ClauseList(), new ClauseList);
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
  D2(fprintf(stderr, "Adding Parent-child-relationship: %s%i -> %s%i\n", sign(parent)?"-":"", var(parent)+1, sign(child)?"-":"", var(child)+1);)
  (*parents)[child]->push_back(parent);
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

Literals* GateAnalyzer::getNextClause(ClauseList* list,
    RootSelectionMethod method) {
  switch (method) {
  case FIRST_CLAUSE: {
    return list->getFirst();
  }
  case MAX_ID: {
    Literal maxLit = mkLit(0, false);
    Literals* result = NULL;
    for (ClauseList::iterator clause = list->begin(); clause != list->end();
        clause++) {
      for (Literals::iterator clit = (*clause)->begin();
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
    Literal min;
    int minOcc = INT_MAX;
    for (int i = 0; i < maxVar(); i++) {
      Literal lit = mkLit(i, false);
      int occ = clauses->countOccurence(lit);
      if (occ != 0 && occ < minOcc) {
        minOcc = occ;
        min = lit;
      }
      occ = clauses->countOccurence(~lit);
      if (occ != 0 && occ < minOcc) {
        minOcc = occ;
        min = ~lit;
      }
    }
    return clauses->getClauses(min)->getFirst();
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
void GateAnalyzer::analyzeEncoding(RootSelectionMethod method, int tries) {
  ClauseList* roots = new ClauseList();
  ClauseList* units = clauses->getByCriteria(createUnitFilter());

  Literal root = mkLit(newVar(), false);
  Literals* unit = new Literals(root);
  clauses->add(unit);
  for (unsigned int i = 0; i < units->size(); i++) {
    clauses->augment(units->get(i), ~root);
  }

  roots->add(unit);
  unit->setMarked();
  analyzeEncoding(root);

  ClauseList* remainder = clauses->getByCriteria(createNoMarkFilter());
  for (int count = 0; count < tries && remainder->size() > 0; count++) {
    Literals* next = getNextClause(remainder, method);

    next->setMarked();
    getOrCreateGate(root)->addForwardClause(next);
    for (Literals::iterator it = next->begin(); it != next->end(); it++) {
      setParent(root, *it);
    }
    clauses->augment(next, ~root);

    for (Literals::iterator it = next->begin(); it != next->end(); it++) {
      if (*it != ~root)
        analyzeEncoding(*it);
    }
    remainder->dumpByCriteria(createMarkFilter());
  }

  for (ClauseList::iterator it = remainder->begin(); it != remainder->end();
      it++) {
    Literals* next = *it;
    next->setMarked();
    getOrCreateGate(root)->addForwardClause(next);
    for (Literals::iterator it2 = next->begin(); it2 != next->end(); it2++) {
      setParent(root, *it2);
    }
    clauses->augment(next, ~root);
  }

  assert(getRoots()->size() == 1 && "now there is exactly one root");
  assert(
      getSideProblem()->size() == 0
          && "all clauses are integrated into the gate-structure");

  delete remainder;
}

/*******
 * Create a DAG starting from a fact. Detect cycles and stop if necessary
 ***/
void GateAnalyzer::analyzeEncoding(Literal root) {
  D1(fprintf(stderr, "Root is %s%i\n", sign(root)?"-":"", var(root)+1);)

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

    ClauseList* fwd = clauses->getClauses(~output)->getByCriteria(
        createNoMarkFilter());
    ClauseList* bwd = clauses->getClauses(output)->getByCriteria(
        createNoMarkFilter());

    D1(
        fprintf(stderr, "Running Gate-Detection on %s%i\n", sign(output)?"-":"", var(output)+1);
        fwd->print(stderr);
        bwd->print(stderr);
        fprintf(stderr, "\n");
    )

    if (fwd->size() > 0 && bwd->isBlockedBy(output, fwd)) {
      if (isMonotonousInput(output)) {
        D1(fprintf(stderr, "Monotonous Parents %s%i\n", sign(output)?"-":"", var(output)+1);)
        gate = defGate(output, fwd, bwd);
      } else { // non-monotonous
        D1(fprintf(stderr, "Non-Monotonous Parents %s%i\n", sign(output)?"-":"", var(output)+1);)
        if (this->full_eq_detection > 0 && bwd->matchesFullGatePattern(output, fwd)) {
          D1(fprintf(stderr, "Gate Detected %s%i\n", sign(output)?"-":"", var(output)+1);)
          gate = defGate(output, fwd, bwd);
          gate->setHasNonMonotonousParent();
        }
        else if (this->full_eq_detection == 2 && isFullGateBySemantic(output, fwd)) {
          D1(fprintf(stderr, "Gate Detected %s%i\n", sign(output)?"-":"", var(output)+1);)
          gate = defGate(output, fwd, bwd);
          gate->setHasNonMonotonousParent();
        }
      }
    }

    if (gate != NULL) {
      literals->insert(literals->end(), gate->getInputs()->begin(),
          gate->getInputs()->end());
    } else {
      delete bwd;
      delete fwd;
    }
  }

  delete literals;
}

bool GateAnalyzer::isLocalFullGate(Literal output, ClauseList* fwd, ClauseList* bwd) {
  if (fwd->size() > 3 || fwd->maxClauseSize() > 4)
      return false;

  MinisatSolver* minisat = new MinisatSolver(bwd);

  int i = 0;
  vector<Literals*> cubes(fwd->size() + 1);
  cubes[i] = new Literals(~output);
  for (Literals* clause : *fwd) {
    Literals* cube = clause->allBut(~output);
    cube->inlineNegate();
    cubes[++i] = cube;
  }

  i = 0;
  vector<int> positions(cubes.size(), 0);
  vector<int> maxima(cubes.size());
  for (Literals* cube : cubes) {
    maxima[i++] = cube->size();
  }

  Literals* lits = new Literals();
  do {
    i = 0;
    for (Literals* cube : cubes) {
      lits->add(cube->get(positions[i++]));
    }
    if (minisat->isUPConsistent(lits)) {
      return false;
    }
  } while (increment(positions, maxima));

  return true;
}

bool GateAnalyzer::isFullGateBySemantic(Literal output, ClauseList* fwd) {
  if (fwd->size() > 3 || fwd->maxClauseSize() > 4)
    return false;

  int i = 0;
  vector<Literals*> cubes(fwd->size() + 1);
  cubes[i] = new Literals(~output);
  for (Literals* clause : *fwd) {
    Literals* cube = clause->allBut(~output);
    cube->inlineNegate();
    cubes[++i] = cube;
  }

  i = 0;
  vector<int> positions(cubes.size(), 0);
  vector<int> maxima(cubes.size());
  for (Literals* cube : cubes) {
    maxima[i++] = cube->size();
  }

  Literals* lits = new Literals();
  do {
    i = 0;
    for (Literals* cube : cubes) {
      lits->add(cube->get(positions[i++]));
    }
    if (minisat->isUPConsistent(lits)) {
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

bool GateAnalyzer::isMonotonousInput(Literal output) {
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

ClauseList* GateAnalyzer::getAllClauses() {
  return clauses;
}

vector<int>* GateAnalyzer::getRecursiveGateOrWidths() {
  vector<int>* widths = new vector<int>(2 * maxVar() + 2, 0);

  vector<bool>* visited = new vector<bool>(2 * maxVar() + 2, false);

  deque<Literal>* literals = new deque<Literal>();
  literals->push_back(getRoots()->getFirst()->getFirst());

  while (!literals->empty()) {
    Literal literal = literals->front();
    literals->pop_front();

    if ((*visited)[toInt(literal)])
      continue;
    (*visited)[toInt(literal)] = true;

    Gate* gate = getGate(literal);
    if (gate == NULL)
      continue;

    for (vector<Literal>::iterator it = gate->getInputs()->begin();
        it != gate->getInputs()->end(); it++) {
      (*widths)[toInt(*it)] = (*widths)[toInt(literal)]
          + gate->countAlternatives(*it);
    }
  }

  return widths;
}

void GateAnalyzer::augmentClauses(int minWidth) {
  int nAugmentedClauses = 0;

  vector<int>* widths = getRecursiveGateOrWidths();

  vector<vector<Literal>*>* dependencies = findConfluentOutputs();

  vector<bool>* visited = new vector<bool>(2 * maxVar() + 2, false);

  deque<Literal>* literals = new deque<Literal>();
  literals->push_back(getRoots()->getFirst()->getFirst());

  while (!literals->empty()) {
    Literal literal = literals->front();
    literals->pop_front();

    if ((*visited)[toInt(literal)])
      continue;
    (*visited)[toInt(literal)] = true;

    Gate* gate = getGate(literal);
    if (gate == NULL)
      continue;

    literals->insert(literals->end(), gate->getInputs()->begin(),
        gate->getInputs()->end());

    ClauseList* fwd = gate->getForwardClauses();
    ClauseList* bwd = gate->getBackwardClauses();
    vector<Literal>* deps = (*dependencies)[toInt(literal)];
    if (deps->size() > 0 && (*widths)[toInt(literal)] >= minWidth) {
      for (ClauseList::iterator it = fwd->begin(); it != fwd->end(); it++) {
        Literals* cl = *it;
        cl->addAll(deps);
        nAugmentedClauses++;
      }
      for (ClauseList::iterator it = bwd->begin(); it != bwd->end(); it++) {
        Literals* cl = *it;
        cl->addAll(deps);
        nAugmentedClauses++;
      }
    }
  }

  fprintf(stderr, "Min Width is %i -> Number of Augmented Clauses: %i\n",
      minWidth, nAugmentedClauses);
}

vector<vector<Literal>*>* GateAnalyzer::findConfluentOutputs() {
  vector<vector<Literal>*>* dependencies = new vector<vector<Literal>*>(
      2 * maxVar() + 2, NULL);
  vector<bool>* visited = new vector<bool>(2 * maxVar() + 2, false);

  deque<Literal>* literals = new deque<Literal>();
  Literal root = getRoots()->getFirst()->getFirst();
  literals->push_back(root);
  (*dependencies)[toInt(root)] = new vector<Literal>();

  vector<Literal>* temp = new vector<Literal>(maxVar());

  while (!literals->empty()) {
    Literal literal = literals->front();
    literals->pop_front();
    (*visited)[toInt(literal)] = true;

    Gate* gate = getGate(literal);
    if (gate == NULL)
      continue;

    // add the intersection of all parents' dependencies as local dependencies
    vector<Literal>* parents = getParents(literal);

    if (parents->size() > 0) {
      vector<Literal>* deps = new vector<Literal>();

      deps->push_back(~(*parents)[0]);
      deps->insert(deps->end(), (*dependencies)[toInt((*parents)[0])]->begin(),
          (*dependencies)[toInt((*parents)[0])]->end());

      for (vector<Literal>::iterator p = parents->begin() + 1;
          p != parents->end(); p++) {
        temp->clear();
        vector<Literal>* pdeps = (*dependencies)[toInt(*p)];

        set_intersection(pdeps->begin(), pdeps->end(), deps->begin(),
            deps->end(), std::back_inserter(*temp));

        deps->clear();
        deps->insert(deps->begin(), temp->begin(), temp->end());
      }

      sort(deps->begin(), deps->end());
      (*dependencies)[toInt(literal)] = deps;
    }

    // only add inputs, that have all parents visited
    vector<Literal>* inputs = gate->getInputs();
    for (vector<Literal>::iterator it = inputs->begin(); it != inputs->end();
        it++) {
      bool allParentsVisited = true;
      vector<Literal>* parents = getParents(*it);
      for (vector<Literal>::iterator it2 = parents->begin();
          it2 != parents->end(); it2++) {
        if (!(*visited)[toInt(*it2)]) {
          allParentsVisited = false;
          break;
        }
      }
      if (allParentsVisited)
        literals->push_back(*it);
    }
  }

  delete literals;

  return dependencies;
}

ClauseList* GateAnalyzer::getSideProblem() {
  ClauseList* list = clauses->getByCriteria(
      createNotFilter(createMarkFilter()));
  D1(printf("SideProblem size = %i (of %i)\n", list->size(), clauses->size());)
  D1(list->print(stderr); fprintf(stderr, "\n");)
  return list;
}

ClauseList* GateAnalyzer::getGateProblem() {
  ClauseList* list = clauses->getByCriteria(createMarkFilter());
  D1(printf("GateProblem size = %i (of %i)\n", list->size(), clauses->size());)
  D1(list->print(stderr); fprintf(stderr, "\n");)
  return list;
}

ClauseList* GateAnalyzer::getPrunedGateProblem(Literals* inputModel) {
  ClauseList* list = new ClauseList();
  FixedSizeLiterals* model = new FixedSizeLiterals(inputModel->size(),
      inputModel);

  vector<bool> visited(2 * inputModel->size(), false);

  // init roots
  ClauseList* roots = clauses->getByCriteria(createUnitFilter());
  vector<Literal>* literals = new vector<Literal>();
  for (unsigned int i = 0; i < roots->size(); i++) {
    Literals* root = roots->get(i);
    list->add(root);
    literals->push_back(root->getFirst());
  }

  // traverse dag
  for (vector<Literal>::iterator it = literals->begin(); it != literals->end();
      it++) {
    Literal literal = *it;
    Gate* gate = getGate(literal);

    if (gate == NULL)
      continue;

    if (model->satisfies(literal) || !gate->isMonotonous()) {
      list->addAll(gate->getForwardClauses());
      vector<Literal>* children = getInputs(literal);
      for (vector<Literal>::iterator child = children->begin();
          child != children->end(); child++) {
        if (!visited[toInt(*child)]) {
          literals->push_back(*child);
          visited[toInt(literal)] = true;
        }
      }
    } else {
    D1(printf("PRUNING AT %s%i\n", sign(literal) ? "-" : "", var(literal)+1);)
  }
}

delete literals;
delete roots;
delete model;

D1(printf("Pruned GateProblem size = %i\n", list->size());)

return list;
}
}
