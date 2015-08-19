/*
 * BlockedClauseDecomposition.h
 *
 *  Created on: Jul 29, 2015
 *      Author: markus
 */

#ifndef TOOLS_BLOCKEDCLAUSEDECOMPOSITION_H_
#define TOOLS_BLOCKEDCLAUSEDECOMPOSITION_H_

namespace Dark {

class ClauseList;
class MappedClauseList;

class BlockedClauseDecomposition {
private:
  ClauseList* clauses;
  MappedClauseList* large;
  MappedClauseList* small;
public:
  BlockedClauseDecomposition(ClauseList* clauses);
  virtual ~BlockedClauseDecomposition();

  void decompose();
  bool isBlockedSet(MappedClauseList* clauses);
  ClauseList* eliminateBlockedClauses(MappedClauseList* clauses);
};

} /* namespace Dark */

#endif /* TOOLS_BLOCKEDCLAUSEDECOMPOSITION_H_ */
