/*
 * ClauseList.cpp
 *
 *  Created on: 13.12.2013
 *      Author: markus
 */

#include "ClauseList.h"
#include "stdio.h"
#include <cstddef>
#include <memory>

namespace Dark {

/**
 * Constructor
 */
ClauseList::ClauseList() {
  clauses = new vector<Clause*>();
}

/**
 * Destructor
 */
ClauseList::~ClauseList() {
  delete clauses;
}

void ClauseList::freeClauses() {
  for (std::vector<Clause*>::iterator it = clauses->begin(); it != clauses->end(); ++it) {
    delete *it;
  }
}

/**
 * Accessing Methods
 */
void ClauseList::add(Clause* clause) {
  clauses->push_back(clause);
}

void ClauseList::addAll(ClauseList* list) {
  for(int i = 0; i < list->size(); i++) {
    this->add(list->get(i));
  }
}

Clause* ClauseList::get(int i) {
  return (*clauses)[i];
}

Clause* ClauseList::getFirst() {
  return (*clauses)[0];
}

Clause* ClauseList::getLast() {
  return (*clauses)[clauses->size()-1];
}

int ClauseList::size() {
  return (int)clauses->size();
}

ClauseList::iterator ClauseList::begin() {
  return clauses->begin();
}

ClauseList::iterator ClauseList::end() {
  return clauses->end();
}

/**
 * Comparative Methods
 */
Clause* ClauseList::find(Clause* clause) {
  for (int i = 0; i < size(); ++i) {
    if (get(i)->equals(clause)) return get(i);
  }
  return NULL;
}

bool ClauseList::contains(Clause* clause) {
  return find(clause) != NULL;
}

ClauseList* ClauseList::getByCriteria(unique_ptr<ClauseFilter> filter) {
  ClauseList* result = new ClauseList();
  for (unsigned int i = 0; i < clauses->size(); i++) {
    if (filter->meetCriteria(this->get(i))) {
      result->add(get(i));
    }
  }
  return result;
}

ClauseList* ClauseList::removeByCriteria(unique_ptr<ClauseFilter> filter) {
  ClauseList* result = new ClauseList();
  vector<Clause*>* nextClauses = new vector<Clause*>();
  for (unsigned int i = 0; i < clauses->size(); i++) {
    if (filter->meetCriteria(this->get(i))) {
      result->add(get(i));
    } else {
      nextClauses->push_back(get(i));
    }
  }
  delete clauses;
  clauses = nextClauses;
  return result;
}

bool ClauseList::isBlockedBy(Literal lit, Clause* clause) {
  for (iterator it = begin(); it != end(); it++) {
    Clause* cl = *it;
    if (!(cl->isBlockedBy(lit, clause))) {
      return false;
    }
  }
  return true;
}

bool ClauseList::isBlockedBy(Literal lit, ClauseList* list) {
  for (iterator it = list->begin(); it != list->end(); it++) {
    Clause* cl = *it;
    if (!(this->isBlockedBy(lit, cl))) {
      return false;
    }
  }
  return true;
}

void ClauseList::unmarkAll() {
  for (unsigned int i = 0; i < clauses->size(); i++) {
    this->get(i)->unsetMarked();
  }
}

/**
 * Output Methods
 */
void ClauseList::print(FILE* out) {
  for (unsigned int i = 0; i < clauses->size(); ++i) {
    Dark::Clause* clause = (*clauses)[i];
    if (clause != NULL) {
      clause->print(out);
    } else {
      fprintf(out, "NULL; ");
    }
  }
}

} /* namespace Analyzer */
