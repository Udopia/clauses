/*
 * ClauseIndex.cc
 *
 *  Created on: Nov 5, 2015
 *      Author: markus
 */

#include "ClauseIndex.h"

#include "ClauseList.h"

namespace Dark {

ClauseIndex::ClauseIndex() {
  clauseMap = new std::map<Literal, Dark::ClauseList*>();
  nVars = 0;
  nClauses = 0;
}

ClauseIndex::ClauseIndex(Dark::ClauseList* clauses) {
  clauseMap = new std::map<Literal, Dark::ClauseList*>();
  nVars = 0;
  nClauses = 0;
  if (clauses != NULL) {
    for (ClauseList::iterator clit = clauses->begin(); clit != clauses->end(); ++clit) {
      this->add(*clit);
    }
  }
}

ClauseIndex::~ClauseIndex() {
  for (std::map<Literal, Dark::ClauseList*>::iterator it = clauseMap->begin(); it != clauseMap->end(); it++) {
    delete it->second;
  }
  delete clauseMap;
}

void ClauseIndex::createVars(Var v) {
  while (nVars <= v) {
    Literal lit = mkLit(nVars++, false);
    (*clauseMap)[lit] = new Dark::ClauseList();
    (*clauseMap)[~lit] = new Dark::ClauseList();
  }
}

void ClauseIndex::add(DynamicLiterals* clause) {
  createVars(clause->maxVar());
  nClauses++;
  for (std::vector<Literal>::iterator lit = clause->begin(); lit != clause->end(); ++lit) {
    (*clauseMap)[*lit]->add(clause);
  }
}

void ClauseIndex::addAll(ClauseList* list) {
  createVars(list->maxVar());
  nClauses += list->size();
  for(ClauseList::iterator clit = list->begin(); clit != list->end(); clit++) {
    for (std::vector<Literal>::iterator lit = (*clit)->begin(); lit != (*clit)->end(); ++lit) {
      (*clauseMap)[*lit]->add(*clit);
    }
  }
}


void ClauseIndex::remove(DynamicLiterals* clause) {
  nClauses--;
  for (std::vector<Literal>::iterator it = clause->begin(); it != clause->end(); ++it) {
    Literal lit = *it;
    (*clauseMap)[lit]->remove(clause);
  }
}

void ClauseIndex::removeAll(ClauseList* list) {
  for(ClauseList::iterator it = list->begin(); it != list->end(); it++) {
    this->remove(*it);
  }
}

void ClauseIndex::augment(DynamicLiterals* clause, Literal lit) {
  createVars(var(lit));
  clause->add(lit);
  (*clauseMap)[lit]->add(clause);
}

void ClauseIndex::augment(ClauseList* clauses, Literal lit) {
  for (DynamicLiterals* clause : *clauses) {
    augment(clause, lit);
  }
}

ClauseList* ClauseIndex::getClauses(Literal literal) {
  return (*clauseMap)[literal];
}

int ClauseIndex::countOccurence(Literal literal) {
  return (*clauseMap)[literal]->size();
}

int ClauseIndex::countVars() {
  return nVars;
}

int ClauseIndex::countClauses() {
  return nClauses;
}

}
