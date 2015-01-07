/*
 * MappedClauseList.cpp
 *
 *  Created on: 13.12.2013
 *      Author: markus
 */

#include "MappedClauseList.h"

#include "Clause.h"

namespace Analyzer {

MappedClauseList::MappedClauseList() {
  clauseMap = new std::map<Literal, Analyzer::ClauseList*>();
  maxVar = 0;
}

MappedClauseList::~MappedClauseList() {
  for (std::map<Literal, Analyzer::ClauseList*>::iterator it = clauseMap->begin(); it != clauseMap->end(); it++) {
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
      (*clauseMap)[lit] = new Analyzer::ClauseList();
    }
    if (clauseMap->count(~lit) <= 0) {
      (*clauseMap)[~lit] = new Analyzer::ClauseList();
    }

    if (maxVar < var(lit)) {
      maxVar = var(lit);
    }

    // fetch and update clause-list
    Analyzer::ClauseList* clauseList = (*clauseMap)[lit];
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
