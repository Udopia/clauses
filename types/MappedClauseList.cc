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
  maxVar = -1;
}

MappedClauseList::~MappedClauseList() {
  for (std::map<Literal, Dark::ClauseList*>::iterator it = clauseMap->begin(); it != clauseMap->end(); it++) {
    delete it->second;
  }
  delete clauseMap;
}

void MappedClauseList::newVar(int var) {
	while (maxVar < var) {
		maxVar++;
		Literal lit = mkLit(maxVar, false);
	  (*clauseMap)[lit] = new Dark::ClauseList();
	  (*clauseMap)[~lit] = new Dark::ClauseList();
	}
}

void MappedClauseList::add(Literals* clause) {
  ClauseList::add(clause);
  // update clause map
  for (std::vector<Literal>::iterator it = clause->begin(); it != clause->end(); ++it) {
    Literal lit = *it;

    newVar(var(lit));

    Dark::ClauseList* clauseList = (*clauseMap)[lit];
    clauseList->add(clause);
  }
}

void MappedClauseList::augment(Literals* clause, Literal lit) {
  clause->add(lit);

  newVar(var(lit));

  // update occurence-list
  (*clauseMap)[lit]->add(clause);
}

void MappedClauseList::augmentAll(Literal lit) {
  newVar(var(lit));

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
  maxVar = -1;

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
