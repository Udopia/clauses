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
#include "../types/ClauseList.h"
#include "../types/MappedClauseList.h"

#include "../filters/ClauseFilters.h"

#include "Projection.h"
#include "GateAnalyzer.h"


#define VERBOSITY 1
#include "../debug.h"

namespace Dark {

GateAnalyzer::GateAnalyzer(ClauseList* clauseList, bool use_refinement) {
  clauses = new MappedClauseList();
  clauses->addAll(clauseList);

  forwardClauses = new map<Literal, ClauseList*>();
  backwardClauses = new map<Literal, ClauseList*>();
  children = new map<Literal, set<Literal>*>();
  parents = new map<Literal, set<Literal>*>();
  projection = new Projection();

  this->use_refinement = use_refinement;

  for (int i = 0; i < clauses->nVars(); i++) {
    (*forwardClauses)[mkLit(i, false)] = new ClauseList();
    (*forwardClauses)[mkLit(i, true)] = new ClauseList();
    (*backwardClauses)[mkLit(i, false)] = new ClauseList();
    (*backwardClauses)[mkLit(i, true)] = new ClauseList();
    (*children)[mkLit(i, false)] = new set<Literal>();
    (*children)[mkLit(i, true)] = new set<Literal>();
    (*parents)[mkLit(i, false)] = new set<Literal>();
    (*parents)[mkLit(i, true)] = new set<Literal>();
  }
}

GateAnalyzer::~GateAnalyzer() {
  freeAllContent();
  delete clauses;
  delete forwardClauses;
  delete backwardClauses;
  delete children;
}

void GateAnalyzer::freeAllContent() {
  clauses->freeClauses();
  for (map<Literal, ClauseList*>::iterator it = forwardClauses->begin(); it != forwardClauses->end(); ++it) {
    delete it->second;
  }

  for (map<Literal, ClauseList*>::iterator it = backwardClauses->begin(); it != backwardClauses->end(); ++it) {
    delete it->second;
  }

  for (map<Literal, set<Literal>*>::iterator it = children->begin(); it != children->end(); ++it) {
    delete it->second;
  }

  for (map<Literal, set<Literal>*>::iterator it = parents->begin(); it != parents->end(); ++it) {
    delete it->second;
  }
  delete forwardClauses;
  delete backwardClauses;
  delete children;
  delete parents;
}


ClauseList* GateAnalyzer::getGateClauses(Literal literal) {
  D2(fprintf(stderr, "Request for Gate Clause of literal %s%i\n", sign(literal)?"-":"", var(literal)+1);
  (*forwardClauses)[literal]->print();
  fprintf(stderr, "\n");)
  return (*forwardClauses)[literal];
}

ClauseList* GateAnalyzer::getRoots() {
  return clauses->getByCriteria(createUnitFilter());
}

set<Literal>* GateAnalyzer::getChildren(Literal parent) {
  return (*children)[parent];
}

bool GateAnalyzer::hasChildren(Literal parent) {
  return getChildren(parent)->size() > 0;
}

set<Literal>* GateAnalyzer::getParents(Literal child) {
  return (*parents)[child];
}

bool GateAnalyzer::hasParents(Literal child) {
  return getParents(child)->size() > 0;
}

void GateAnalyzer::setParentChild(Literal parent, Literal child) {
  D2(fprintf(stderr, "Adding Parent-child-relationship: %s%i -> %s%i\n", sign(parent)?"-":"", var(parent)+1, sign(child)?"-":"", var(child)+1);)

  (*children)[parent]->insert(child);
  (*parents)[child]->insert(parent);
}

void GateAnalyzer::unsetParentChild(Literal parent) {
  fprintf(stderr, "unset parent %s%i, ", sign(parent)?"-":"", var(parent)+1);
  for (set<Literal>::iterator it = (*children)[parent]->begin(); it != (*children)[parent]->end(); it++) {
    Literal child = *it;
    fprintf(stderr, "child %s%i, ", sign(*it)?"-":"", var(*it)+1);
    (*parents)[child]->erase(parent);
  }
  D1(
  for (set<Literal>::iterator it = (*children)[parent]->begin(); it != (*children)[parent]->end(); it++) {
    Literal child = *it;
    for (set<Literal>::iterator it2 = (*parents)[child]->begin(); it2 != (*parents)[child]->end(); it2++) {
      fprintf(stderr, "%s%i, ", sign(*it2)?"-":"", var(*it2)+1);
    }
  }
  )
  delete (*children)[parent];
  (*children)[parent] = new set<Literal>();
}

void GateAnalyzer::analyzeEncoding() {
  ClauseList* roots = getRoots();

  for (int i = 0; i < roots->size(); i++) {
    Literal root = roots->get(i)->getFirst();
    roots->get(i)->setMarked(); // visited
    analyzeEncoding(root);
  }

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

  D1(fprintf(stderr, "\n\n");)

  delete literals;
}

/**
 * Locally analyze encoding of current literal and mark clauses as visited if a gate was detected.
 * If we have full equivalence encoding on literal it marks the back-pointers as such.
 */
bool GateAnalyzer::classifyEncoding(Literal output) {
  if (projectionContains(var(output))) {
    D1(fprintf(stderr, "Stop on projection member %s%i\n", sign(output)?"-":"", var(output)+1);)
    return false;
  } else {
    D1(fprintf(stderr, "Literal is %s%i\n", sign(output)?"-":"", var(output)+1);)
  }

  if (hasParents(~output)) {
    D1(fprintf(stderr, "Literal %s%i already has parents\n", sign(~output)?"-":"", var(~output)+1);)

    if (this->use_refinement) {
      // try to fix it
      if (getParents(~output)->size() == 1) {
        Literal parent = *(getParents(~output)->begin());
        D1(fprintf(stderr, "Found one parent %s%i\n\n", sign(parent)?"-":"", var(parent)+1);)
        if ((*forwardClauses)[parent]->size() == 1) {
          D1(fprintf(stderr, "########Found one forward clause. Fixing it.\n");)
          unsetParentChild(parent);
          fprintf(stderr, "Unregister gate clauses: \n");
          (*forwardClauses)[parent]->print(stderr);
          (*backwardClauses)[parent]->print(stderr);
          (*forwardClauses)[parent]->unmarkAll();
          (*backwardClauses)[parent]->unmarkAll();
          delete (*forwardClauses)[parent];
          (*forwardClauses)[parent] = new ClauseList();
          delete (*backwardClauses)[parent];
          (*backwardClauses)[parent] = new ClauseList();
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
  D1(fprintf(stderr, "Running Gate-Detection on %s%i\n", sign(output)?"-":"", var(output)+1);
  fprintf(stderr, "Checking if ClauseList [");
  backward->print(stderr);
  fprintf(stderr, "] is blocked by [");
  forward->print(stderr);
  fprintf(stderr, "]\n");)

  // check if all remaining incoming clauses are back-implications (like in full-equivalence encoding after Tseitin)
  if (!backward->isBlockedBy(output, forward)) {
    D1(fprintf(stderr, "Gate-Detection Failed\n");)
    return false;
  } else {
    D1(fprintf(stderr, "Gate-Detection Successful\n");)
  }

  if (forward->size() > 0) {
    D1(fprintf(stderr, "Storing Gate %s%i:\n", sign(output)?"-":"", var(output)+1);)
    D1(forward->print(stderr);)
    D1(fprintf(stderr, "\n");)
    setAsGate(output, forward);
  }

  for (ClauseList::iterator it = backward->begin(); it != backward->end(); it++) {
    Dark::Clause* clause = *it;
    (*backwardClauses)[output]->add(clause);
    clause->setMarked();
  }

  return true;
}

void GateAnalyzer::setAsGate(Literal output, Dark::ClauseList* definition) {
  for (ClauseList::iterator it = definition->begin(); it != definition->end(); it++) {
    Dark::Clause* clause = *it;
    (*forwardClauses)[output]->add(clause);
    clause->setMarked();

    // set parent-child relationship
    for (std::vector<Literal>::iterator lit = clause->begin(); lit != clause->end(); ++lit) {
      if (*lit != ~output) setParentChild(output, *lit);
    }
  }
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

    if (forwardClauses->count(literal) > 0) {
      D2(fprintf(stderr, "Found Gate Clauses\n");)
      if (model->satisfies(literal) || !isMonotonous(literal)) {
        D2(fprintf(stderr, "Adding them\n");)
        list->addAll((*forwardClauses)[literal]);
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
    } else if (getChildren(literal) != NULL && getChildren(literal)->size() > 0) {
      fprintf(stderr, "*!*!*!*!*!* Inconsistent Data-Structures *!*!*!*!*!*\n");
    }
  }
  D1(fprintf(stderr, "Pruned at literals: ");
//  std::for_each(prunedLits->begin(), prunedLits->end(), [this](Literal n){ fprintf(stderr, "%s%i, ", sign(n) ? "-" : "", var(n)+1); });
  for (set<Literal>::iterator it = prunedLits->begin(); it != prunedLits->end(); it++) {
    fprintf(stderr, "%s%i, ", sign(*it) ? "-" : "", var(*it)+1);
  }
  fprintf(stderr, "\n");)

  delete literals;
  delete prunedLits;
  delete roots;
  delete model;

  D1(printf("Pruned GateProblem size = %i\n", list->size());)

  return list;
}
}
