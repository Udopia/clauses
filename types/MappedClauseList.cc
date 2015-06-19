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
  maxVar = 0;
}

MappedClauseList::~MappedClauseList() {
  for (std::map<Literal, Dark::ClauseList*>::iterator it = clauseMap->begin(); it != clauseMap->end(); it++) {
    delete it->second;
  }
  delete clauseMap;
}

void MappedClauseList::add(Literals* clause) {
  ClauseList::add(clause);
  // update clause map
  for (std::vector<Literal>::iterator it = clause->begin(); it != clause->end(); ++it) {
    Literal lit = *it;

    // create missing clause-list for both Literalerals
    if (clauseMap->count(lit) <= 0) {
      (*clauseMap)[lit] = new Dark::ClauseList();
    }
    if (clauseMap->count(~lit) <= 0) {
      (*clauseMap)[~lit] = new Dark::ClauseList();
    }

    if (maxVar < var(lit)) {
      maxVar = var(lit);
    }

    // fetch and update clause-list
    Dark::ClauseList* clauseList = (*clauseMap)[lit];
    clauseList->add(clause);
  }
}

void MappedClauseList::augment(Literals* clause, Literal lit) {
  clause->add(lit);

  // create missing clause-list for both literals
  if (clauseMap->count(lit) <= 0) {
    (*clauseMap)[lit] = new Dark::ClauseList();
  }
  if (clauseMap->count(~lit) <= 0) {
    (*clauseMap)[~lit] = new Dark::ClauseList();
  }

  if (maxVar < var(lit)) {
    maxVar = var(lit);
  }

  // update occurence-list
  (*clauseMap)[lit]->add(clause);
}

void MappedClauseList::augmentAll(Literal lit) {
  // create missing clause-list for both Literals
  if (clauseMap->count(lit) <= 0) {
    (*clauseMap)[lit] = new Dark::ClauseList();
  }
  if (clauseMap->count(~lit) <= 0) {
    (*clauseMap)[~lit] = new Dark::ClauseList();
  }

  if (maxVar < var(lit)) {
    maxVar = var(lit);
  }

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
  clauses = nextClauses;

  delete clauseMap;
  clauseMap = new std::map<Literal, Dark::ClauseList*>();
  maxVar = 0;

  for (vector<Literals*>::iterator it = clauses->begin(); it != clauses->end(); it++) {
    Literals* clause = *it;
    for (std::vector<Literal>::iterator it = clause->begin(); it != clause->end(); ++it) {
      Literal lit = *it;

      // create missing clause-list for both Literals
      if (clauseMap->count(lit) <= 0) {
        (*clauseMap)[lit] = new Dark::ClauseList();
      }
      if (clauseMap->count(~lit) <= 0) {
        (*clauseMap)[~lit] = new Dark::ClauseList();
      }

      if (maxVar < var(lit)) {
        maxVar = var(lit);
      }

      // fetch and update clause-list
      Dark::ClauseList* clauseList = (*clauseMap)[lit];
      clauseList->add(clause);
    }
  }
}

ClauseList* MappedClauseList::getClauses(Literal literal) {
  return (*clauseMap)[literal];
}

int MappedClauseList::countOccurence(Literal literal) {
  return (*clauseMap)[literal]->size();
}

} /* namespace Analyzer */
