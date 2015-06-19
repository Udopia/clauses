/*
 * MappedClauseList.h
 *
 *  Created on: 13.12.2013
 *      Author: markus
 */

#ifndef MAPPEDCLAUSELIST_H_
#define MAPPEDCLAUSELIST_H_

#include <map>

#include "ClauseList.h"
#include "MappedClauseList.h"

namespace Dark {

class Literals;

class MappedClauseList: public ClauseList {
private:
  std::map<Literal, Dark::ClauseList*>* clauseMap;

public:
  MappedClauseList();
  virtual ~MappedClauseList();

  void add(Literals* clause);

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

} /* namespace Analyzer */
#endif /* MAPPEDCLAUSELIST_H_ */
