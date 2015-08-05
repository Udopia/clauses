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
  this->large = NULL;
  this->small = NULL;
}

BlockedClauseDecomposition::~BlockedClauseDecomposition() {
}

void BlockedClauseDecomposition::decompose() {
  this->large = new ClauseList();
  this->large->addAll(this->clauses);
  this->small = this->large->removeByCriteria(createUnitFilter());
}

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
        //cindex.makeBlockingLiteralFirst(clause, lit);
        blockedClauses->add(clause);
        for (Literals::iterator iter = clause->begin(); iter != clause->end(); iter++) {
          Literal l = *iter;
          if (notInStack[toInt(~l)]) {
            litsToCheck->push(~l);
            notInStack[toInt(~l)] = false;
          }
        }
        clauses->remove(clause);
      }
    }
  }

  return blockedClauses;
}

} /* namespace Dark */
