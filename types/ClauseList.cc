/*
 * ClauseList.cpp
 *
 *  Created on: 13.12.2013
 *      Author: markus
 */

#include "ClauseList.h"
#include "Literal.h"
#include "stdio.h"
#include "limits.h"
#include <cstddef>
#include <memory>

namespace Dark {

/**
 * Constructor
 */
ClauseList::ClauseList() {
  clauses = new vector<Literals*>();
  max_var = -1;
}

/**
 * Destructor
 */
ClauseList::~ClauseList() {
  delete clauses;
}

void ClauseList::freeClauses() {
  for (std::vector<Literals*>::iterator it = clauses->begin(); it != clauses->end(); ++it) {
    delete *it;
  }
}

/**
 * Accessing Methods
 */
void ClauseList::add(Literals* clause) {
  if (clause->maxVar() > this->maxVar()) max_var = clause->maxVar();
  clauses->push_back(clause);
}

void ClauseList::remove(Literals* clause) {
  (*clauses)[pos(clause)] = getLast();
  clauses->pop_back();
}

void ClauseList::addAll(ClauseList* list) {
  for(ClauseList::iterator it = list->begin(); it != list->end(); it++) {
    this->add(*it);
  }
}

Literals* ClauseList::get(int i) {
  return (*clauses)[i];
}

Literals* ClauseList::getFirst() {
  return (*clauses)[0];
}

Literals* ClauseList::getLast() {
  return (*clauses)[clauses->size()-1];
}

unsigned int ClauseList::size() {
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
int ClauseList::pos(Literals* clause) {
  for (unsigned int i = 0; i < size(); ++i) {
    if (get(i) == clause) return i;
  }
  return -1;
}

Literals* ClauseList::find(Literals* clause) {
  for (unsigned int i = 0; i < size(); ++i) {
    if (get(i)->equals(clause)) return get(i);
  }
  return NULL;
}

bool ClauseList::contains(Literals* clause) {
  return find(clause) != NULL;
}

ClauseList* ClauseList::getByCriteria(unique_ptr<ClauseFilter> filter) {
  ClauseList* result = new ClauseList();
  for (vector<Literals*>::iterator it = clauses->begin(); it != clauses->end(); it++) {
    if (filter->meetCriteria(*it)) {
      result->add(*it);
    }
  }
  return result;
}

ClauseList* ClauseList::removeByCriteria(unique_ptr<ClauseFilter> filter) {
  ClauseList* result = new ClauseList();
  vector<Literals*>* nextClauses = new vector<Literals*>();
  for (vector<Literals*>::iterator it = clauses->begin(); it != clauses->end(); it++) {
    if (filter->meetCriteria(*it)) {
      result->add(*it);
    } else {
      nextClauses->push_back(*it);
    }
  }
  delete clauses;
  clauses = nextClauses;
  return result;
}

void ClauseList::dumpByCriteria(unique_ptr<ClauseFilter> filter) {
  vector<Literals*>* nextClauses = new vector<Literals*>();
  for (vector<Literals*>::iterator it = clauses->begin(); it != clauses->end(); it++) {
    if (!filter->meetCriteria(*it)) {
      nextClauses->push_back(*it);
    }
  }
  delete clauses;
  clauses = nextClauses;
}

bool ClauseList::isBlockedBy(Literal lit, Literals* clause) {
  for (iterator it = begin(); it != end(); it++) {
    Literals* cl = *it;
    if (!(cl->isBlockedBy(lit, clause))) {
      return false;
    }
  }
  return true;
}

bool ClauseList::isBlockedBy(Literal lit, ClauseList* list) {
  for (iterator it = list->begin(); it != list->end(); it++) {
    Literals* cl = *it;
    if (!(this->isBlockedBy(lit, cl))) {
      return false;
    }
  }
  return true;
}

bool ClauseList::matchesFullGatePattern(Literal lit, ClauseList* other) {
  Literals* thisLits = this->getUnionOfLiterals();
  Literals* otherLits = other->getUnionOfLiterals();
  otherLits->inlineNegate();
  if (!thisLits->equals(otherLits)) {
    return false;
  }

  // fast and-/or-detection
  if (this->size() == 1 && other->size() == otherLits->size()-1) {
    return other->maxClauseSize() == other->minClauseSize() && other->maxClauseSize() == 2;
  }
  if (other->size() == 1 && this->size() == thisLits->size()-1) {
    return this->maxClauseSize() == this->minClauseSize() && this->maxClauseSize() == 2;
  }

  return false;
}

Literals* ClauseList::getUnionOfLiterals() {
  Literals* clause = new Literals();
  for (ClauseList::iterator it = this->begin(); it != this->end(); it++) {
    for (Literals::iterator it2 = (*it)->begin(); it2 != (*it)->end(); it2++) {
      if (!clause->contains(*it2)) {
        clause->add(*it2);
      }
    }
  }
  return clause;
}

unsigned int ClauseList::minClauseSize() {
  unsigned int min = INT_MAX;
  for (ClauseList::iterator it = this->begin(); it != this->end(); it++) {
    if (min > (*it)->size()) min = (*it)->size();
  }
  return min;
}

unsigned int ClauseList::maxClauseSize() {
  unsigned int max = 0;
  for (ClauseList::iterator it = this->begin(); it != this->end(); it++) {
    if (max < (*it)->size()) max = (*it)->size();
  }
  return max;
}

void ClauseList::markAll() {
  for (unsigned int i = 0; i < clauses->size(); i++) {
    this->get(i)->setMarked();
  }
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
    Dark::Literals* clause = (*clauses)[i];
    if (clause != NULL) {
      clause->print(out);
    } else {
      fprintf(out, "NULL; ");
    }
  }
}

void ClauseList::printDimacs(FILE* out) {
  for (unsigned int i = 0; i < clauses->size(); ++i) {
    Dark::Literals* clause = (*clauses)[i];
    if (clause != NULL) {
      clause->printDimacs(out);
    } else {
      fprintf(out, "0");
    }
  }
}


} /* namespace Analyzer */
