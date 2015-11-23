/*
 * BlockedClauseDecomposition.cc
 *
 *  Created on: Jul 29, 2015
 *      Author: markus
 */

#include "BlockedClauseDecomposition.h"

#include <climits>
#include <stdio.h>
#include <stdlib.h>
#include <deque>
#include <stack>
#include <cassert>

#include "../filters/ClauseFilters.h"

#include "../types/Literal.h"
#include "../types/Literals.h"
#include "../types/ClauseList.h"
#include "../types/ClauseIndex.h"

#define VERBOSITY 0
#include "../misc/debug.h"

namespace Dark {

BlockedClauseDecomposition::BlockedClauseDecomposition(ClauseList* clauses) {
  this->clauses = clauses;
  this->index = new ClauseIndex(clauses);
  large = NULL;
  small = NULL;
}

BlockedClauseDecomposition::~BlockedClauseDecomposition() {
}

/**
 * Perform unit decomposition followed by pure decomposition
 */
void BlockedClauseDecomposition::decompose() {
  small = new ClauseList();
  large = new ClauseList();
  large->addAll(this->clauses);

  // Unit Decomposition
  D1(printf("unit decompose");)
  ClauseList* units = large->removeByCriteria(createUnitFilter());
  index->removeAll(units);
  small->addAll(units);

  D1(printf("up simplify");)
  // Remove UP satisfied clauses from the large set
  for (ClauseList::iterator unit = small->begin(); unit != small->end(); unit++) {
    Literal lit = (*unit)->getFirst();
    ClauseList* remove = index->getClauses(lit);
    large->removeAll(remove);
    index->removeAll(remove);
  }

  D1(printf("check blocked");)
  if (isBlockedSet(large, index)) {
    return;
  }

  D1(printf("pure decompose");)
  // Pure Decomposition
  for (int var = 0; var < large->maxVar(); var++) {
    ClauseList* pos = index->getClauses(mkLit(var, false));
    ClauseList* neg = index->getClauses(mkLit(var, true));
    ClauseList* transfer = new ClauseList();
    if (pos->size() > neg->size()) {
      transfer = neg;
    } else {
      transfer = pos;
    }
    small->addAll(transfer);
    large->removeAll(transfer);
    index->removeAll(transfer);
  }
}

/**
 * Eager Mover: try to move blocks of clauses at once from the small set to the large set
 */
void BlockedClauseDecomposition::postprocess() {
  // sort clauses in the small set according to the resolution connections in the large set
  map<Literals*, int>* clauseScores = new map<Literals*, int>();
  for (ClauseList::iterator clause_it = small->begin(); clause_it != small->end(); clause_it++) {
    int score = 0;
    for (Literals::iterator lit_it = (*clause_it)->begin(); lit_it != (*clause_it)->end(); lit_it++) {
      score += index->countOccurence(~(*lit_it));
    }
    (*clauseScores)[*clause_it] = score;
  }
  small->sort(clauseScores);

  // try moving blocks of clauses from small to large
  ClauseList* newLarge = new ClauseList();
  ClauseIndex* newIndex = new ClauseIndex(newLarge);
  newLarge->addAll(large);
  bool success = true;
  int nSlices = 4;
  while (success) {
    success = false;
    for (int slice = 0; slice < nSlices; slice++) {
      int blockSize = small->size() / nSlices;
      unsigned int from = slice * blockSize;
      unsigned int to = from + blockSize;
      if (to > small->size()) to = small->size();
      ClauseList* block = small->slice(from, to);
      newLarge->addAll(block);
      newIndex->addAll(block);
      if (!isBlockedSet(newLarge, newIndex)) {
        newLarge->removeAll(block);
        newIndex->removeAll(block);
      } else {
        small->removeAll(block);
        success = true;
        slice--;
      }
    }
  }
  delete large;
  large = newLarge;
}

void BlockedClauseDecomposition::shiftSmallByUnit() {
  Var v = clauses->newVar();
  ClauseIndex* index = new ClauseIndex(small);
  index->augment(small, mkLit(v, true));
  large->addAll(small);
  delete small;
  small = new ClauseList();
  Literals* unit = new Literals(mkLit(v, false));
  small->add(unit);
  clauses->add(unit);
}

bool BlockedClauseDecomposition::isBlockedSet(ClauseList* clauses, ClauseIndex* index) {
  ClauseList* blocked = eliminateBlockedClauses(clauses, index);
  bool isBlocked = (clauses->size() == 0);
  clauses->addAll(blocked);
  delete blocked;
  return isBlocked;
}

/**
 * based on SimplifiedArminsBCEliminator
 */
ClauseList* BlockedClauseDecomposition::eliminateBlockedClauses(ClauseList* clauses, ClauseIndex* index) {
  std::stack<Literal>* litsToCheck = new std::stack<Literal>();
  vector<bool> notInStack(2 + 2 * clauses->maxVar(), false);

  for (int i = 1; i <= clauses->maxVar(); i++) {
    litsToCheck->push(mkLit(i, false));
    litsToCheck->push(mkLit(i, true));
  }

  ClauseList* blockedClauses = new ClauseList();

  while (litsToCheck->size() > 0) {
    Literal lit = litsToCheck->top();
    litsToCheck->pop();
    notInStack[toInt(lit)] = true;

    ClauseList* occurance = index->getClauses(lit);
    ClauseList* mirror = index->getClauses(~lit);

    D1(printf("check lit %s%i\n", sign(lit) ? "-" : "", var(lit)+1);)
    D2(printf("check if clauses: "); occurance->print();)
    D2(printf(" are blocked by clauses "); mirror->print();)
    D2(printf("\n");)

    for (ClauseList::iterator it = occurance->begin(); it != occurance->end(); it++) {
      Literals* clause = *it;

      if (clause == NULL) {
        printf("clause is null");
        continue;
      }

      if (mirror->size() == 0 || mirror->isBlockedBy(lit, clause)) {
        // move blocking literal to the beginning of the list
        D1(printf("clause is blocked: "); clause->println();)
        (*clause)[clause->pos(lit)] = (*clause)[0];
        (*clause)[0] = lit;
        // move blocked clause to separate list
        blockedClauses->add(clause);
        clauses->remove(clause);
        index->remove(clause);
        // push all literals to the stack
        for (Literals::iterator iter = clause->begin(); iter != clause->end(); iter++) {
          Literal l = *iter;
          if (notInStack[toInt(~l)]) {
            litsToCheck->push(~l);
            notInStack[toInt(~l)] = false;
          }
        }
      }
    }
  }

  return blockedClauses;
}

} /* namespace Dark */
