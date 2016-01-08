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
#include "../types/ClauseList.h"
#include "../types/ClauseIndex.h"
#include "../types/PooledLiterals.h"

#define VERBOSITY 0
#include "../misc/debug.h"

namespace Dark {

BlockedClauseDecomposition::BlockedClauseDecomposition(ClauseList* clauses) {
  small = new ClauseList();
  large = clauses;
  index = new ClauseIndex(large);
}

BlockedClauseDecomposition::~BlockedClauseDecomposition() {
}

/**
 * Perform unit decomposition followed by pure decomposition
 */
void BlockedClauseDecomposition::decompose() {
  // Unit Decomposition
  D1(printf("unit decompose\n");)
  ClauseList* units = large->removeByCriteria(createUnitFilter());
  index->removeAll(units);
  small->addAll(units);

  // Remove UP satisfied clauses from the large set
  D1(printf("up simplify\n");)
  for (ClauseList::iterator unit = small->begin(); unit != small->end(); unit++) {
    ClauseList* remove = index->getClauses(*((*unit)->begin()));
    large->removeAll(remove);
    index->removeAll(remove);
  }

  D1(printf("check blocked\n");)
  if (isBlockedSet(index)) {
    return;
  }

  D1(printf("pure decompose\n");)
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
  map<PooledLiterals*, int>* clauseScores = new map<PooledLiterals*, int>();
  for (ClauseList::iterator clause_it = small->begin(); clause_it != small->end(); clause_it++) {
    int score = 0;
    for (PooledLiterals::iterator lit_it = (*clause_it)->begin(); *lit_it != litFalse; lit_it++) {
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
      unsigned int to = std::min(from + blockSize, small->size());
      ClauseList* block = small->slice(from, to);
      newIndex->addAll(block);
      if (!isBlockedSet(newIndex)) {
        newIndex->removeAll(block);
      } else {
        newLarge->addAll(block);
        small->removeAll(block);
        success = true;
        slice--;
      }
      delete block;
    }
  }
  delete large;
  large = newLarge;
}

void BlockedClauseDecomposition::shiftSmallByUnit() {
  Var v = small->newVar();
  ClauseIndex* index = new ClauseIndex(small);
  index->augment(small, mkLit(v, true));
  large->addAll(small);
  delete small;
  small = new ClauseList();
  PooledLiterals* unit = new PooledLiterals(mkLit(v, false));
  small->add(unit);
}

bool BlockedClauseDecomposition::isBlockedSet(ClauseIndex* index) {
  ClauseList* blocked = eliminateBlockedClauses(index);
  bool isBlocked = (index->countClauses() == 0);
  index->addAll(blocked);
  delete blocked;
  return isBlocked;
}

/**
 * based on SimplifiedArminsBCEliminator
 */
ClauseList* BlockedClauseDecomposition::eliminateBlockedClauses(ClauseIndex* index) {
  std::stack<Literal>* litsToCheck = new std::stack<Literal>();
  vector<bool> notInStack(2 + 2 * index->countVars(), false);

  for (int i = 1; i <= index->countVars(); i++) {
    litsToCheck->push(mkLit(i, false));
    litsToCheck->push(mkLit(i, true));
  }

  ClauseList* blockedClauses = new ClauseList();

  while (litsToCheck->size() > 0) {
    Literal lit = litsToCheck->top();
    litsToCheck->pop();
    notInStack[toInt(lit)] = true;

    ClauseList* occurrence = new ClauseList(index->getClauses(lit));
    ClauseList* mirror = index->getClauses(~lit);

    for (ClauseList::iterator it = occurrence->begin(); it != occurrence->end(); it++) {
      PooledLiterals* clause = *it;

      if (mirror->size() == 0 || mirror->isBlockedBy(lit, clause)) {
        // move blocking literal to the beginning of the list
        int pos = clause->pos(lit);
        (*clause)[pos] = (*clause)[0];
        (*clause)[0] = lit;
        // move blocked clause to separate list
        blockedClauses->add(clause);
        index->remove(clause);
        // push all literals to the stack
        for (PooledLiterals::iterator iter = clause->begin(); *iter != litFalse; iter++) {
          Literal l = *iter;
          if (notInStack[toInt(~l)]) {
            litsToCheck->push(~l);
            notInStack[toInt(~l)] = false;
          }
        }
      }
    }
    delete occurrence;
  }

  delete litsToCheck;

  return blockedClauses;
}

ClauseList* BlockedClauseDecomposition::decomposeClauses() {
  decompose();
  postprocess();
  shiftSmallByUnit();
  large->addAll(small);
  return large;
}

} /* namespace Dark */
