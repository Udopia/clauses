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
  while (nVars <= v+2) {
    Literal lit = mkLit(nVars++, false);
    (*clauseMap)[lit] = new Dark::ClauseList();
    (*clauseMap)[~lit] = new Dark::ClauseList();
  }
}

void ClauseIndex::add(PooledLiterals* clause) {
  nClauses++;
  for (PooledLiterals::iterator lit = clause->begin(); lit != clause->end(); ++lit) {
    createVars(var(*lit));
    if ((*clauseMap)[*lit] == NULL) {
      printf("Literal %s%i; Max Variable %i", sign(*lit)?"-":"", var(*lit), nVars);
      exit(1);
    }
    (*clauseMap)[*lit]->add(clause);
  }
}

void ClauseIndex::addAll(ClauseList* list) {
  for(ClauseList::iterator clit = list->begin(); clit != list->end(); clit++) {
    this->add(*clit);
  }
}


void ClauseIndex::remove(PooledLiterals* clause) {
  nClauses--;
  for (PooledLiterals::iterator it = clause->begin(); it != clause->end(); ++it) {
    Literal lit = *it;
    (*clauseMap)[lit]->remove(clause);
  }
}

void ClauseIndex::removeAll(ClauseList* list) {
  for(ClauseList::iterator it = list->begin(); it != list->end(); it++) {
    this->remove(*it);
  }
}

void ClauseIndex::augment(PooledLiterals* clause, Literal lit) {
  createVars(var(lit));
  clause->add(lit);
  (*clauseMap)[lit]->add(clause);
}

void ClauseIndex::augment(ClauseList* clauses, Literal lit) {
  for (PooledLiterals* clause : *clauses) {
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
