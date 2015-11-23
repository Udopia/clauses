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
class ClauseIndex;

class BlockedClauseDecomposition {
private:
  ClauseList* large;
  ClauseList* small;
  ClauseIndex* index;

  bool isBlockedSet(ClauseIndex* index);
  ClauseList* eliminateBlockedClauses(ClauseIndex* index);

public:
  BlockedClauseDecomposition(ClauseList* clauses);
  virtual ~BlockedClauseDecomposition();

  void decompose();
  void postprocess();
  void shiftSmallByUnit();

  ClauseList* decomposeClauses();

};

} /* namespace Dark */

#endif /* TOOLS_BLOCKEDCLAUSEDECOMPOSITION_H_ */
