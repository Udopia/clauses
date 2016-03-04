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
#include <algorithm>

namespace Dark {

/**
 * Constructor
 */
ClauseList::ClauseList() {
  clauses = new vector<PooledLiterals*>();
  max_var = 0;
}

ClauseList::ClauseList(ClauseList* list) {
  clauses = new vector<PooledLiterals*>();
  max_var = 0;
  if (list != NULL) {
    this->addAll(list);
  }
}

ClauseList::ClauseList(std::vector<PooledLiterals*>* vec) {
  if (vec != NULL) {
    clauses = vec;
  } else {
    clauses = new vector<PooledLiterals*>();
  }
  max_var = 0;
}

/**
 * Destructor
 */
ClauseList::~ClauseList() {
  delete clauses;
}

void ClauseList::freeClauses() {
  for (std::vector<PooledLiterals*>::iterator it = clauses->begin(); it != clauses->end(); ++it) {
    delete *it;
  }
}

void ClauseList::add(PooledLiterals* clause) {
  max_var = std::max(max_var, clause->maxVar());
  clauses->push_back(clause);
}

void ClauseList::remove(PooledLiterals* clause) {
  (*clauses)[pos(clause)] = getLast();
  clauses->pop_back();
}

void ClauseList::addAll(ClauseList* list) {
  max_var = std::max(max_var, list->max_var);
  clauses->insert(clauses->end(), list->clauses->begin(), list->clauses->end());
}

void ClauseList::removeAll(ClauseList* list) {
  for(ClauseList::iterator it = list->begin(); it != list->end(); it++) {
    this->remove(*it);
  }
}

PooledLiterals* ClauseList::get(int i) {
  return (*clauses)[i];
}

PooledLiterals* ClauseList::getFirst() {
  return (*clauses)[0];
}

PooledLiterals* ClauseList::getLast() {
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

ClauseList* ClauseList::slice(unsigned int from, unsigned int to) {
  if (to <= from) {
    return new ClauseList();
  } else if (to >= this->size()) {
    return new ClauseList(this->clauses);
  } else {
    return new ClauseList(new vector<PooledLiterals*>(clauses->begin() + from, clauses->begin() + (to - from)));
  }
}

// sort clauses by given score
void ClauseList::sort(map<PooledLiterals*, int>* clauseScore) {
  struct comparator {
    comparator(map<PooledLiterals*, int>* key) : key(key) {};
    bool operator() (PooledLiterals* a, PooledLiterals* b) {
      return (*key)[a] < (*key)[b];
    }
    map<PooledLiterals*, int>* key;
  };
  std::sort(clauses->begin(), clauses->end(), comparator(clauseScore));
}

/**
 * Comparative Methods
 */
int ClauseList::pos(PooledLiterals* clause) {
  for (unsigned int i = 0; i < size(); ++i) {
    if (get(i) == clause) return i;
  }
  return -1;
}

PooledLiterals* ClauseList::find(PooledLiterals* clause) {
  for (unsigned int i = 0; i < size(); ++i) {
    if (get(i)->equals(clause)) return get(i);
  }
  return NULL;
}

bool ClauseList::contains(PooledLiterals* clause) {
  return find(clause) != NULL;
}

ClauseList* ClauseList::getByCriteria(unique_ptr<ClauseFilter> filter) {
  ClauseList* result = new ClauseList();
  for (vector<PooledLiterals*>::iterator it = clauses->begin(); it != clauses->end(); it++) {
    if (filter->meetCriteria(*it)) {
      result->add(*it);
    }
  }
  return result;
}

ClauseList* ClauseList::removeByCriteria(unique_ptr<ClauseFilter> filter) {
  ClauseList* result = new ClauseList();
  vector<PooledLiterals*>* nextClauses = new vector<PooledLiterals*>();
  for (vector<PooledLiterals*>::iterator it = clauses->begin(); it != clauses->end(); it++) {
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
  vector<PooledLiterals*>* nextClauses = new vector<PooledLiterals*>();
  for (vector<PooledLiterals*>::iterator it = clauses->begin(); it != clauses->end(); it++) {
    if (!filter->meetCriteria(*it)) {
      nextClauses->push_back(*it);
    }
  }
  delete clauses;
  clauses = nextClauses;
}

bool ClauseList::isBlockedBy(Literal blocking, PooledLiterals* clause) {
  for (iterator it = begin(); it != end(); it++) {
    PooledLiterals* cl = *it;
    if (!(cl->isBlockedBy(blocking, clause))) {
      return false;
    }
  }
  return true;
}

bool ClauseList::isBlockedBy(Literal lit, ClauseList* list) {
  for (iterator it = list->begin(); it != list->end(); it++) {
    if (!(this->isBlockedBy(lit, *it))) {
      return false;
    }
  }
  return true;
}

bool ClauseList::matchesFullGatePattern(Literal lit, ClauseList* other) {
  DynamicLiterals* thisLits = this->getUnionOfLiterals();
  DynamicLiterals* otherLits = other->getUnionOfLiterals();
  otherLits->inlineNegate();
  if (!thisLits->equals(otherLits)) {
    delete thisLits;
    delete otherLits;
    return false;
  }

  // fast and-/or-detection
  bool result = false;
  if (this->size() == 1 && other->size() == otherLits->size()-1) {
    result = other->maxClauseSize() == other->minClauseSize() && other->maxClauseSize() == 2;
  }
  if (!result && other->size() == 1 && this->size() == thisLits->size()-1) {
    result = this->maxClauseSize() == this->minClauseSize() && this->maxClauseSize() == 2;
  }
  if (!result && this->size() == 2 && other->size() == 2 && thisLits->size() == 5 && otherLits->size() == 5) {
    result = this->maxClauseSize() == this->minClauseSize() && this->maxClauseSize() == 3;
  }

  delete thisLits;
  delete otherLits;
  return result;
}

DynamicLiterals* ClauseList::getUnionOfLiterals() {
  DynamicLiterals* clause = new DynamicLiterals();
  for (ClauseList::iterator it = this->begin(); it != this->end(); it++) {
    for (PooledLiterals::iterator it2 = (*it)->begin(); *it2 != litFalse; it2++) {
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
    Dark::PooledLiterals* clause = (*clauses)[i];
    if (clause != NULL) {
      clause->print(out);
      fprintf(out, "; ");
    } else {
      fprintf(out, "NULL; ");
    }
  }
}

void ClauseList::printDimacs(FILE* out) {
  fprintf(out, "p cnf %i %i\n", maxVar()+1, size());
  for (unsigned int i = 0; i < clauses->size(); ++i) {
    Dark::PooledLiterals* clause = (*clauses)[i];
    if (clause != NULL) {
      clause->printDimacs(out);
    } else {
      fprintf(out, "0");
    }
  }
}


} /* namespace Analyzer */
