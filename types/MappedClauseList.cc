/*
 * MappedClauseList.cpp
 *
 *  Created on: 13.12.2013
 *      Author: markus
 */

#include "MappedClauseList.h"
#include "Literals.h"

namespace Dark {

MappedClauseList::MappedClauseList() {
  clauseMap = new std::map<Literal, Dark::ClauseList*>();
  max_var = -1;
}

MappedClauseList::~MappedClauseList() {
  for (std::map<Literal, Dark::ClauseList*>::iterator it = clauseMap->begin(); it != clauseMap->end(); it++) {
    delete it->second;
  }
  delete clauseMap;
}

void MappedClauseList::add(Literals* clause) {
  addVarsUntil(clause->maxVar());
  ClauseList::add(clause);
  // update clause map
  for (std::vector<Literal>::iterator it = clause->begin(); it != clause->end(); ++it) {
    Literal lit = *it;
    (*clauseMap)[lit]->add(clause);
  }
}

void MappedClauseList::remove(Literals* clause) {
  ClauseList::remove(clause);
  // update clause map
  for (std::vector<Literal>::iterator it = clause->begin(); it != clause->end(); ++it) {
    Literal lit = *it;
    (*clauseMap)[lit]->remove(clause);
  }
}

void MappedClauseList::augment(Literals* clause, Literal lit) {
  clause->add(lit);
  addVarsUntil(var(lit));
  // update occurence-list
  (*clauseMap)[lit]->add(clause);
}

void MappedClauseList::augmentAll(Literal lit) {
  addVarsUntil(var(lit));

  for (iterator it = begin(); it != end(); it++) {
    Literals* cl = *it;
    cl->add(lit);
    (*clauseMap)[lit]->add(cl);
  }
}

void MappedClauseList::dumpByCriteria(unique_ptr<ClauseFilter> filter) {
  vector<Literals*>* nextClauses = new vector<Literals*>();

  for (vector<Literals*>::iterator it = clauses->begin(); it != clauses->end(); it++) {
    if (!filter->meetCriteria(*it)) {
      nextClauses->push_back(*it);
    }
  }

  delete clauses;
  delete clauseMap;

  clauses = new vector<Literals*>();
  clauseMap = new std::map<Literal, Dark::ClauseList*>();
  max_var = -1;

  for (vector<Literals*>::iterator it = nextClauses->begin(); it != nextClauses->end(); it++) {
    this->add(*it);
  }

  delete nextClauses;
}

ClauseList* MappedClauseList::getClauses(Literal literal) {
  return (*clauseMap)[literal];
}

int MappedClauseList::countOccurence(Literal literal) {
  return (*clauseMap)[literal]->size();
}

} /* namespace Analyzer */
