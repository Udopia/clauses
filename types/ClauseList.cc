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
  for(ClauseList::iterator it = list->begin(); it != list->end(); it++) {
    this->add(*it);
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
Clause* ClauseList::find(Clause* clause) {
  for (unsigned int i = 0; i < size(); ++i) {
    if (get(i)->equals(clause)) return get(i);
  }
  return NULL;
}

bool ClauseList::contains(Clause* clause) {
  return find(clause) != NULL;
}

ClauseList* ClauseList::getByCriteria(unique_ptr<ClauseFilter> filter) {
  ClauseList* result = new ClauseList();
  for (vector<Clause*>::iterator it = clauses->begin(); it != clauses->end(); it++) {
    if (filter->meetCriteria(*it)) {
      result->add(*it);
    }
  }
  return result;
}

ClauseList* ClauseList::removeByCriteria(unique_ptr<ClauseFilter> filter) {
  ClauseList* result = new ClauseList();
  vector<Clause*>* nextClauses = new vector<Clause*>();
  for (vector<Clause*>::iterator it = clauses->begin(); it != clauses->end(); it++) {
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
  vector<Clause*>* nextClauses = new vector<Clause*>();
  for (vector<Clause*>::iterator it = clauses->begin(); it != clauses->end(); it++) {
    if (!filter->meetCriteria(*it)) {
      nextClauses->push_back(*it);
    }
  }
  delete clauses;
  clauses = nextClauses;
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

bool ClauseList::definesEquivalence(Literal lit, ClauseList* other) {
//  fprintf(stderr, "full detection:");
  if (!isBlockedBy(lit, other)) {
//    fprintf(stderr, "!blocked\n");
    return false;
  }
  Clause* thisLits = this->getUnionOfLiterals();
  Clause* otherLits = other->getUnionOfLiterals();
  otherLits->inlineNegate();
  if (!thisLits->equals(otherLits)) {
//    fprintf(stderr, "!samevars\n");
    return false;
  }

//  fprintf(stderr, "This Size: %i lits:%i; min/max: %i/%i\n", this->size(), thisLits->size(), this->minClauseSize(), this->maxClauseSize());
//  fprintf(stderr, "Other Size: %i lits:%i; min/max: %i/%i\ņ", other->size(), otherLits->size(), other->minClauseSize(), other->maxClauseSize());

  // fast and-/or-detection
  if (this->size() == 1 && other->size() == otherLits->size()-1) {
//    fprintf(stderr, "!sizes fit\n");
    return other->maxClauseSize() == other->minClauseSize() && other->maxClauseSize() == 2;
  }
  if (other->size() == 1 && this->size() == thisLits->size()-1) {
//    fprintf(stderr, "!sizes fit\n");
    return this->maxClauseSize() == this->minClauseSize() && this->maxClauseSize() == 2;
  }

  // TODO: equivalence detection
//  fprintf(stderr, "!hit\n");
  return false;
}

Clause* ClauseList::getUnionOfLiterals() {
  Clause* clause = new Clause();
  for (ClauseList::iterator it = this->begin(); it != this->end(); it++) {
    for (Clause::iterator it2 = (*it)->begin(); it2 != (*it)->end(); it2++) {
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
    Dark::Clause* clause = (*clauses)[i];
    if (clause != NULL) {
      clause->print(out);
    } else {
      fprintf(out, "NULL; ");
    }
  }
}

void ClauseList::printDimacs(FILE* out) {
  for (unsigned int i = 0; i < clauses->size(); ++i) {
    Dark::Clause* clause = (*clauses)[i];
    if (clause != NULL) {
      clause->printDimacs(out);
    } else {
      fprintf(out, "0");
    }
  }
}


} /* namespace Analyzer */
