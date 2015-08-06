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

#include "../filters/ClauseFilters.h"

#include "../types/Literal.h"
#include "../types/Literals.h"
#include "../types/ClauseList.h"
#include "../types/MappedClauseList.h"

namespace Dark {

BlockedClauseDecomposition::BlockedClauseDecomposition(ClauseList* clauses) {
  this->clauses = clauses;
  large = NULL;
  small = NULL;
}

BlockedClauseDecomposition::~BlockedClauseDecomposition() {
}

void BlockedClauseDecomposition::decompose() {
  large = new MappedClauseList();
  large->addAll(this->clauses);

  // Unit Decomposition
  small = large->removeByCriteria(createUnitFilter());

  // Check Blocked State
  ClauseList* blocked = eliminateBlockedClauses(large);
  if (large->size() == 0) { // done
    delete large;
    large = blocked;
    return;
  }
  large->addAll(blocked); // revert
  delete blocked;

  // Remove clauses that are satisfied by unit-prop.
  for (ClauseList::iterator unit = small->begin(); unit != small->end(); unit++) {
    Literal lit = (*unit)->getFirst();
    while (large->getClauses(lit)->size() > 0) {
      large->remove(large->getClauses(lit)->getLast());
    }
  }

  // Check Blocked State
  ClauseList* blocked = eliminateBlockedClauses(large);
  if (large->size() == 0) { // done
    delete large;
    large = blocked;
    return;
  }
  large->addAll(blocked); // revert
  delete blocked;

  // TODO: Pure Decomposition
}


/**
 * based on SimplifiedArminsBCEliminator
 */
ClauseList* BlockedClauseDecomposition::eliminateBlockedClauses(MappedClauseList* clauses) {
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
    ClauseList* occurance = clauses->getClauses(lit);
    for (ClauseList::iterator it = occurance->begin(); it != occurance->end(); it++) {
      Literals* clause = *it;
      if (clauses->getClauses(~lit)->isBlockedBy(lit, clause)) {
        // move blocking literal to the beginning of the list
        (*clause)[clause->pos(lit)] = (*clause)[0];
        (*clause)[0] = lit;
        // move blocked clause to separate list
        blockedClauses->add(clause);
        clauses->remove(clause);
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