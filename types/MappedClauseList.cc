/*
 * MappedClauseList.cpp
 *
 *  Created on: 13.12.2013
 *      Author: markus
 */

#include "MappedClauseList.h"

#include "Clause.h"

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

void MappedClauseList::add(Clause* clause) {
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

ClauseList* MappedClauseList::getClauses(Literal literal) {
  return (*clauseMap)[literal];
}

int MappedClauseList::countOccurence(Literal literal) {
  return (*clauseMap)[literal]->size();
}

} /* namespace Analyzer */
