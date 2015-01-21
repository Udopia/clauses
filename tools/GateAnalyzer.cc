/*
 * SolverMetaInfo.cc
 *
 *  Created on: 04.05.2013
 *      Author: markus
 */

#include <climits>
#include <stdio.h>
#include <stdlib.h>

#include "../types/Literal.h"
#include "../types/Cube.h"
#include "../types/FixedSizeCube.h"
#include "../types/Clause.h"
#include "../types/MappedClauseList.h"

#include "../filters/ClauseFilters.h"

#include "Gate.h"
#include "Projection.h"
#include "GateAnalyzer.h"

#include <assert.h>

#define VERBOSITY 0
#include "../debug.h"

namespace Dark {

GateAnalyzer::GateAnalyzer(ClauseList* clauseList, bool use_refinement) {
  clauses = new MappedClauseList();
  clauses->addAll(clauseList);

  parents = new map<Literal, vector<Literal>*>();
  projection = new Projection();

  this->use_refinement = use_refinement;

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
  for (map<Literal, vector<Literal>*>::iterator it = parents->begin(); it != parents->end(); ++it) {
    delete it->second;
  }
}

ClauseList* GateAnalyzer::getGateClauses(Literal literal) {
  Gate* gate = (*gates)[var(literal)];
  if (gate == NULL || gate->getOutput() != literal) return NULL;
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
    assert (gate == NULL && "gate must not exist");
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
  for (vector<Literal>::iterator it = inputs->begin(); it != inputs->end(); it++) {
    Literal child = *it;
    vector<Literal>* outputs = (*parents)[child];
    outputs->erase(std::remove(outputs->begin(), outputs->end(), parent), outputs->end());
  }
}

/***
 * If there is a unit-clause, just start from there.
 * If there are more than one unit-clauses, add a new variable and create a new and-gate from the existing unit-clauses.
 * If there are no unit-clauses, select a clause and add a unit-clause that implies the selected clause.
 */
void GateAnalyzer::analyzeEncoding() {
  ClauseList* roots = getRoots();
  Literal root;

  if (roots->size() == 1) {
    roots->get(0)->setMarked();
    root = roots->get(0)->getFirst();
  }
  else if (roots->size() > 1) {
    root = mkLit(newVar(), false);
    Clause* unit = new Clause(root);
    clauses->add(unit);
    unit->setMarked();
    for (unsigned int i = 0; i < roots->size(); i++) {
      clauses->augment(roots->get(i), ~root);
    }
  }
  else if (roots->size() == 0) {
    // heuristic selects maximum variable
    root = mkLit(maxVar(), false);
    if (clauses->countOccurence(root) == 0) {
      root = ~root;
    }
    assert(clauses->countOccurence(root) != 0 && "clause must exist");
    delete roots;
    roots = clauses->getClauses(root);
    root = mkLit(newVar(), false);
    Clause* unit = new Clause(root);
    clauses->add(unit);
    unit->setMarked();
    for (unsigned int i = 0; i < roots->size(); i++) {
      clauses->augment(roots->get(i), ~root);
    }
  }

  analyzeEncoding(root);

  delete roots;
}

/***
 * The more general approach.
 * Analyze until all clauses are part of the hierarchy.
 * Select Clauses by the following heuristic:
 * 1. first select all unit-clauses
 * 2. if there is still a side-problem and no unit-clause left, select the one containing the biggest variable
 * 3. in the end forge together all selected root-clauses by one unit that implies them all
 */
void GateAnalyzer::analyzeEncoding2(RootSelectionMethod method, int tries) {
  ClauseList* roots = new ClauseList();
  ClauseList* units = clauses->getByCriteria(createUnitFilter());

  Literal root = mkLit(newVar(), false);
  Clause* unit = new Clause(root);
  clauses->add(unit);
  for (unsigned int i = 0; i < units->size(); i++) {
    clauses->augment(units->get(i), ~root);
  }

  roots->add(unit);
  unit->setMarked();
  analyzeEncoding(root);

  ClauseList* remainder = clauses->getByCriteria(createNoMarkFilter());
  for (int count = 0; count < tries && remainder->size() > 0; count++) {
    Clause* next = getNextClause(remainder, method);

    next->setMarked();
    getOrCreateGate(root)->addForwardClause(next);
    for (Clause::iterator it = next->begin(); it != next->end(); it++) {
      setParent(root, *it);
    }
    clauses->augment(next, ~root);

    for (Clause::iterator it = next->begin(); it != next->end(); it++) {
      if (*it != ~root) analyzeEncoding(*it);
    }
    remainder->dumpByCriteria(createMarkFilter());
  }

  for (ClauseList::iterator it = remainder->begin(); it != remainder->end(); it++) {
    Clause* next = *it;
    next->setMarked();
    getOrCreateGate(root)->addForwardClause(next);
    for (Clause::iterator it2 = next->begin(); it2 != next->end(); it2++) {
      setParent(root, *it2);
    }
    clauses->augment(next, ~root);
  }

  assert (getRoots()->size() == 1 && "now there is exactly one root");
  assert (getSideProblem()->size() == 0 && "all clauses are integrated into the gate-structure");

  delete remainder;
}

Clause* GateAnalyzer::getNextClause(ClauseList* list, RootSelectionMethod method) {
  switch (method) {
  case FIRST_CLAUSE: {
    return list->getFirst();
  }
  case MAX_ID: {
    Literal maxLit = mkLit(0, false);
    Clause* result = NULL;
    for (ClauseList::iterator clause = list->begin(); clause != list->end(); clause++) {
      for (Clause::iterator clit = (*clause)->begin(); clit != (*clause)->end(); clit++) {
        if (var(*clit) > var(maxLit)) {
          maxLit = *clit;
          result = *clause;
        }
      }
    }
    return result;
  }
  case MIN_OCCURENCE: {
    vector<int>* occurence = new vector<int>(maxVar(), 0);
    vector<Clause*>* occCl = new vector<Clause*>(maxVar(), NULL);
    for (ClauseList::iterator clause = list->begin(); clause != list->end(); clause++) {
      for (Clause::iterator clit = (*clause)->begin(); clit != (*clause)->end(); clit++) {
        (*occurence)[var(*clit)]++;
        (*occCl)[var(*clit)] = *clause;
      }
    }
    int var = occurence->size() - 1;
    while ((*occurence)[var] == 0) var--;
    for (int i = 0, n = var; i < n; i++) {
      if ((*occurence)[i] == 0) continue;
      if ((*occurence)[var] > (*occurence)[i]) {
        var = i;
      }
    }
    return (*occCl)[var];
  }
  default: return list->getLast();
  }
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
    Literal lit = (*literals)[i_lit];

    // Stop on projection (forced inputs)
    if (projectionContains(var(lit))) {
      continue;
    }

    // Stop on possible emerging cycles
    if (hasParents(~lit)) {
      // We can try to fix it
      if (this->use_refinement && countParents(~lit) == 1) {
        Literal parent = *(getParents(~lit)->begin());
        Gate* gate = (*gates)[var(parent)];
        if (gate->getForwardClauses()->size() == 1) {
          D1(fprintf(stderr, "########Found one forward clause. Fixing it.\n");)
          unsetParent(parent);
          gate->getForwardClauses()->unmarkAll();
          gate->getBackwardClauses()->unmarkAll();
          delete gate;
          (*gates)[var(parent)] = NULL;
          if (!isGate(lit)) {
            D1(fprintf(stderr, "######Refinement Failed. Redo old stuff.\n");)
            isGate(parent);
            continue;
          }
        }
      } else {
        continue;
      }
    }

    if (isGate(lit)) {
      // Append inputs of the newly discovered gate to the queue
      vector<Literal>* children = getInputs(lit);
      literals->insert(literals->end(), children->begin(), children->end());
    }
  }

  delete literals;
}

/*******
 * Locally analyze encoding of current literal and mark clauses as visited if a gate was detected.
 * If we have full equivalence encoding on literal it marks the back-pointers as such.
 ***/
bool GateAnalyzer::isGate(Literal output) {
  D1(fprintf(stderr, "Running Gate-Detection on %s%i\n", sign(output)?"-":"", var(output)+1);)

  ClauseList* backward = clauses->getClauses(output)->getByCriteria(createNotFilter(createMarkFilter()));
  ClauseList* forward = clauses->getClauses(~output)->getByCriteria(createNotFilter(createMarkFilter()));


  D2(fprintf(stderr, "Checking if ClauseList [");
    backward->print(stderr);
    fprintf(stderr, "] is blocked by [");
    forward->print(stderr);
    printf(stderr, "]\n");)

  if (forward->size() == 0) {
    D2(fprintf(stderr, "Stop on %s%i as there is no forward clause\n", sign(output)?"-":"", var(output)+1);)
    delete backward;
    delete forward;
    return false;
  }

  // check if all remaining incoming clauses are back-implications (like in full-equivalence encoding after Tseitin)
  if (!backward->isBlockedBy(output, forward)) {
    delete backward;
    delete forward;
    return false;
  }

  forward->markAll();
  backward->markAll();
  Gate* gate = new Gate(output, forward, backward);
  (*gates)[var(output)] = gate;

  vector<Literal>* inputs = gate->getInputs();
  for (std::vector<Literal>::iterator lit = inputs->begin(); lit != inputs->end(); ++lit) {
    setParent(output, *lit);
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

ClauseList* GateAnalyzer::getPGClauses() {
  assert (getRoots()->size() == 1 && "at that point there should be exactly one output");

  ClauseList* pgClauses = new ClauseList();
  pgClauses->addAll(getRoots());
  for (vector<Gate*>::iterator it = gates->begin(); it != gates->end(); it++) {
    Gate* gate = *it;
    if (gate != NULL) {
      pgClauses->addAll(gate->getForwardClauses());

      vector<Literal>* p = getParents(gate->getOutput());
      for (vector<Literal>::iterator it = p->begin(); it != p->end(); it++) {
        Gate* pGate = getGate(*it);
        if (pGate != NULL && !pGate->isMonotonous()) {
          pgClauses->addAll(gate->getBackwardClauses());
        }
      }
    }
  }
  pgClauses->addAll(getSideProblem());
  return pgClauses;
}

ClauseList* GateAnalyzer::getSideProblem() {
  ClauseList* list = clauses->getByCriteria(createNotFilter(createMarkFilter()));
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

ClauseList* GateAnalyzer::getPrunedGateProblem(Cube* inputModel) {
  ClauseList* list = new ClauseList();
  FixedSizeCube* model = new FixedSizeCube(inputModel->size(), inputModel);

  vector<bool> visited(2*inputModel->size(), false);

  // init roots
  ClauseList* roots = clauses->getByCriteria(createUnitFilter());
  vector<Literal>* literals = new vector<Literal>();
  for (unsigned int i = 0; i < roots->size(); i++) {
    Clause* root = roots->get(i);
    list->add(root);
    literals->push_back(root->getFirst());
  }

  // traverse dag
  for (vector<Literal>::iterator it = literals->begin(); it != literals->end(); it++) {
    Literal literal = *it;
    Gate* gate = getGate(literal);

    if (gate == NULL) continue;

    if (model->satisfies(literal) || !gate->isMonotonous()) {
      list->addAll(gate->getForwardClauses());
      vector<Literal>* children = getInputs(literal);
      for (vector<Literal>::iterator child = children->begin(); child != children->end(); child++) {
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
