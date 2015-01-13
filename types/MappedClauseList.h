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

class Clause;

class MappedClauseList: public ClauseList {
private:
  std::map<Literal, Dark::ClauseList*>* clauseMap;
  int maxVar;

public:
  MappedClauseList();
  virtual ~MappedClauseList();

  void add(Clause* clause);
  ClauseList* getClauses(Literal literal);
  ClauseList* getFiltered(Literal literal, bool stripVisited, bool stripBackpointers);

  int nVars() {
    return this->maxVar + 1;
  }

  int countOccurence(Literal literal);
};

} /* namespace Analyzer */
#endif /* MAPPEDCLAUSELIST_H_ */
