/*
 * ClauseIndex.h
 *
 *  Created on: Oct 31, 2015
 *      Author: markus
 */

#ifndef TYPES_CLAUSEINDEX_H_
#define TYPES_CLAUSEINDEX_H_


#include <vector>
#include <memory>
#include "Literals.h"
#include "Literal.h"
#include "../filters/ClauseFilter.h"

using namespace std;

namespace Dark {

class ClauseIndex {
private:
  std::map<Literal, Dark::ClauseList*>* clauseMap;

public:
  ClauseIndex();
  virtual ~ClauseIndex();

  void add(Literals* clause);
  void remove(Literals* clause);

  void augment(Literals* clause, Literal lit);
  void augmentAll(Literal lit);

  int newVar() {
    Literal lit = mkLit(++max_var, false);
    (*clauseMap)[lit] = new Dark::ClauseList();
    (*clauseMap)[~lit] = new Dark::ClauseList();
    return max_var;
  }

  void addVarsUntil(Var v) {
    while (max_var < v) {
      newVar();
    }
  }

  ClauseList* getClauses(Literal literal);
  ClauseList* getFiltered(Literal literal, bool stripVisited, bool stripBackpointers);

  void dumpByCriteria(unique_ptr<ClauseFilter> filter);

  int countOccurence(Literal literal);
};

}

#endif /* TYPES_CLAUSEINDEX_H_ */
