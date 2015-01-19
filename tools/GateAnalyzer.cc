/*
 * SolverMetaInfo.cc
 *
 *  Created on: 04.05.2013
 *      Author: markus
 */


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

  children = new map<Literal, set<Literal>*>();
  parents = new map<Literal, set<Literal>*>();
  projection = new Projection();

  this->use_refinement = use_refinement;

  for (int i = 0; i < clauses->nVars(); i++) {
    newVar();
  }

  gates = new vector<Gate*>(clauses->nVars(), NULL);
}

GateAnalyzer::~GateAnalyzer() {
  freeAllContent();
  delete clauses;
  delete children;
  delete parents;
}

void GateAnalyzer::freeAllContent() {
  clauses->freeClauses();
  for (map<Literal, set<Literal>*>::iterator it = children->begin(); it != children->end(); ++it) {
    delete it->second;
  }
  for (map<Literal, set<Literal>*>::iterator it = parents->begin(); it != parents->end(); ++it) {
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
  (*children)[mkLit(max_var, false)] = new set<Literal>();
  (*children)[mkLit(max_var, true)] = new set<Literal>();
  (*parents)[mkLit(max_var, false)] = new set<Literal>();
  (*parents)[mkLit(max_var, true)] = new set<Literal>();
  return max_var;
}

ClauseList* GateAnalyzer::getRoots() {
  return clauses->getByCriteria(createUnitFilter());
}

set<Literal>* GateAnalyzer::getChildren(Literal parent) {
  return (*children)[parent];
}

int GateAnalyzer::countChildren(Literal parent) {
  return getChildren(parent)->size();
}

bool GateAnalyzer::hasChildren(Literal parent) {
  return countChildren(parent) > 0;
}

set<Literal>* GateAnalyzer::getParents(Literal child) {
  return (*parents)[child];
}

int GateAnalyzer::countParents(Literal child) {
  return getParents(child)->size();
}

bool GateAnalyzer::hasParents(Literal child) {
  return countParents(child) > 0;
}

void GateAnalyzer::setParentChild(Literal parent, Literal child) {
  D2(fprintf(stderr, "Adding Parent-child-relationship: %s%i -> %s%i\n", sign(parent)?"-":"", var(parent)+1, sign(child)?"-":"", var(child)+1);)
  (*children)[parent]->insert(child);
  (*parents)[child]->insert(parent);
}

void GateAnalyzer::unsetParentChild(Literal parent) {
  D2(fprintf(stderr, "unset parent %s%i, ", sign(parent)?"-":"", var(parent)+1);)
  for (set<Literal>::iterator it = (*children)[parent]->begin(); it != (*children)[parent]->end(); it++) {
    Literal child = *it;
    D2(fprintf(stderr, "child %s%i, ", sign(*it)?"-":"", var(*it)+1);)
    (*parents)[child]->erase(parent);
  }
  delete (*children)[parent];
  (*children)[parent] = new set<Literal>();
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
    for (int i = 0; i < roots->size(); i++) {
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
    for (int i = 0; i < roots->size(); i++) {
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
void GateAnalyzer::analyzeEncoding2() {
  ClauseList* roots = new ClauseList();
  D1(fprintf(stderr, "*** %d CLAUSES TOTAL\n", clauses->size());)

  ClauseList* units = clauses->getByCriteria(createUnitFilter());

  D1(fprintf(stderr, "*** %d UNITS\n", units->size());)

  for (int i = 0; i < units->size(); i++) {
    Clause* unit = units->get(i);
    if (unit->isMarked()) continue;
    unit->setMarked();
    roots->add(unit);
    analyzeEncoding(unit->getFirst());
  }
  delete units;

  ClauseList* remainder = clauses->getByCriteria(createNoMarkFilter());
  while (remainder->size() != 0) {
    D1(fprintf(stderr, "*** %d CLAUSES REMAINING\n", remainder->size());)
    // select new root
    Clause* next = remainder->get(0);
    next->setMarked();
    roots->add(next);
    for (unsigned int i = 0; i < next->size(); i++) {
      analyzeEncoding(next->get(i));
    }
    delete remainder;
    remainder = clauses->getByCriteria(createNoMarkFilter());
  }
  delete remainder;

  Literal root = mkLit(newVar(), false);
  Clause* unit = new Clause(root);
  clauses->add(unit);
  unit->setMarked();
  for (unsigned int i = 0; i < roots->size(); i++) {
    clauses->augment(roots->get(i), ~root);
  }

  assert (getRoots()->size() == 1 && "now there is exactly one root");

  delete roots;
}


/*********************************
 * Create a DAG starting from a fact. Detect cycles and stop if necessary
 */
void GateAnalyzer::analyzeEncoding(Literal root) {
  D1(fprintf(stderr, "Root is %s%i\n", sign(root)?"-":"", var(root)+1);)

  // a queue of literals that grows while checking for implicates
  vector<Literal>* literals = new vector<Literal>();
  literals->push_back(root);

  // while there are literals in the queue we check for children and add them to the queue
  for (unsigned int i_lit = 0; i_lit < literals->size(); i_lit++) {
    Literal parent = (*literals)[i_lit];

    // Locally analyze encoding of current literal.
    if (!classifyEncoding(parent)) {
      continue;
    }

    // Add literals that are involved in the gate-definition to the queue
    set<Literal>* children = getChildren(parent);
    for (set<Literal>::iterator it = children->begin(); it != children->end(); it++) {
      Literal child = *it;
      literals->push_back(child);
    }
  }

  delete literals;
}

/**
 * Locally analyze encoding of current literal and mark clauses as visited if a gate was detected.
 * If we have full equivalence encoding on literal it marks the back-pointers as such.
 */
bool GateAnalyzer::classifyEncoding(Literal output) {
  if (projectionContains(var(output))) {
    D2(fprintf(stderr, "Stop on projection member %s%i\n", sign(output)?"-":"", var(output)+1);)
    return false;
  } else {
    D2(fprintf(stderr, "Literal is %s%i\n", sign(output)?"-":"", var(output)+1);)
  }

  if (hasParents(~output)) {
    D2(fprintf(stderr, "Literal %s%i already has parents\n", sign(~output)?"-":"", var(~output)+1);)

    if (this->use_refinement) {
      // try to fix it
      if (countParents(~output) == 1) {
        Literal parent = *(getParents(~output)->begin());
        D1(fprintf(stderr, "Found one parent %s%i\n\n", sign(parent)?"-":"", var(parent)+1);)
        Gate* gate = (*gates)[var(parent)];
        if (gate->getForwardClauses()->size() == 1) {
          D1(fprintf(stderr, "########Found one forward clause. Fixing it.\n");)
          unsetParentChild(parent);
          gate->getForwardClauses()->unmarkAll();
          gate->getBackwardClauses()->unmarkAll();
          delete gate;
          if (!classifyEncoding(output)) {
            D1(fprintf(stderr, "######Refinement Failed. Redo old stuff.\n");)
            classifyEncoding(parent);
            return false;
          } else {
            return true;
          }
        }
      }
    }
    return false;
  }

  bool result = false;

  ClauseList* backward = clauses->getClauses(output)->getByCriteria(createNotFilter(createMarkFilter()));
  ClauseList* forward = clauses->getClauses(~output)->getByCriteria(createNotFilter(createMarkFilter()));

  if (forward->size() == 0) {
    D2(fprintf(stderr, "Stop on %s%i as there is no forward clause\n", sign(output)?"-":"", var(output)+1);)
    result = false;
  }
  else if (isGate(output, forward, backward)) {
    result = true;
  }

  delete backward;
  delete forward;

  return result;
}

bool GateAnalyzer::isGate(Literal output, Dark::ClauseList* forward, Dark::ClauseList* backward) {
  D1(fprintf(stderr, "Running Gate-Detection on %s%i\n", sign(output)?"-":"", var(output)+1);)
  D2(fprintf(stderr, "Checking if ClauseList [");
    backward->print(stderr);
    fprintf(stderr, "] is blocked by [");
    forward->print(stderr);
    printf(stderr, "]\n");)

  // check if all remaining incoming clauses are back-implications (like in full-equivalence encoding after Tseitin)
  if (!backward->isBlockedBy(output, forward)) {
    D2(fprintf(stderr, "Gate-Detection Failed\n");)
    return false;
  } else {
    D2(fprintf(stderr, "Gate-Detection Successful\n");)
  }

  if (forward->size() > 0) {
    D2(fprintf(stderr, "Storing Gate %s%i:\n", sign(output)?"-":"", var(output)+1);
    forward->print(stderr);
    fprintf(stderr, "\n");)

    for (ClauseList::iterator it = forward->begin(); it != forward->end(); it++) {
      (*it)->setMarked();
    }

    for (ClauseList::iterator it = backward->begin(); it != backward->end(); it++) {
      (*it)->setMarked();
    }

    Gate* gate = new Gate(output, forward, backward);
    (*gates)[var(output)] = gate;

    vector<Literal>* inputs = gate->getInputs();
    for (std::vector<Literal>::iterator lit = inputs->begin(); lit != inputs->end(); ++lit) {
      setParentChild(output, *lit);
    }
  }

  return true;
}

bool GateAnalyzer::isMonotonous(Literal literal) {
  set<Literal>* children = getChildren(literal);
  for (set<Literal>::iterator child = children->begin(); child != children->end(); child++) {
    if (find(children->begin(), children->end(), ~(*child)) != children->end()) {
      return false;
    }
  }
  return true;
}

ClauseList* GateAnalyzer::getNotMarkedClauses() {
  return clauses->getByCriteria(createNotFilter(createMarkFilter()));
}



void GateAnalyzer::setProjection(Projection* projection) {
  this->projection = projection;
}

void GateAnalyzer::removeProjection() {
  this->projection = NULL;
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
  return clauses;
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
  getGateProblem();

  ClauseList* list = new ClauseList();

  FixedSizeCube* model = new FixedSizeCube(inputModel->size(), inputModel);

  set<Literal>* prunedLits = new set<Literal>();

  vector<bool> visited(2*inputModel->size(), false);

  // init roots
  ClauseList* roots = clauses->getByCriteria(createUnitFilter());
  Literals* literals = new Literals();
  for (int i = 0; i < roots->size(); i++) {
    Clause* root = roots->get(i);
    list->add(root);
    literals->add(root->getFirst());
  }

  // traverse dag
  for (unsigned int i = 0; i < literals->size(); i++) {
    Literal literal = literals->get(i);
    D2(fprintf(stderr, "Literal is %s%i\n", sign(~literal)?"-":"", var(~literal)+1);)

    Gate* gate = (*gates)[var(literal)];
    if (gate != NULL && gate->getOutput() == literal) {
      D2(fprintf(stderr, "Found Gate Clauses\n");)
      if (model->satisfies(literal) || !isMonotonous(literal)) {
        D2(fprintf(stderr, "Adding them\n");)
        list->addAll(gate->getForwardClauses());
        set<Literal>* children = getChildren(literal);
        for (set<Literal>::iterator child = children->begin(); child != children->end(); child++) {
          if (!visited[toInt(*child)]) {
            literals->add(*child);
            visited[toInt(literal)] = true;
          }
        }
      }
      else {
        D2(fprintf(stderr, "Pruning them\n");)
        D1(prunedLits->insert(literal);
        printf("################ PRUNING AT %s%i\n", sign(literal) ? "-" : "", var(literal)+1);)
      }
    }
  }
  D1(fprintf(stderr, "Pruned at literals: ");
  std::for_each(prunedLits->begin(), prunedLits->end(), [this](Literal n){ fprintf(stderr, "%s%i, ", sign(n) ? "-" : "", var(n)+1); });
  fprintf(stderr, "\n");)

  delete literals;
  delete prunedLits;
  delete roots;
  delete model;

  D1(printf("Pruned GateProblem size = %i\n", list->size());)

  return list;
}
}
