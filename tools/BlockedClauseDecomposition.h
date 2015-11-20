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
  ClauseList* clauses;
  ClauseList* large;
  ClauseList* small;
  ClauseIndex* index;
public:
  BlockedClauseDecomposition(ClauseList* clauses);
  virtual ~BlockedClauseDecomposition();

  void decompose();
  void postprocess();
  void shiftSmallByUnit();
  bool isBlockedSet(ClauseList* clauses, ClauseIndex* index);
  ClauseList* eliminateBlockedClauses(ClauseList* clauses, ClauseIndex* index);
};

} /* namespace Dark */

#endif /* TOOLS_BLOCKEDCLAUSEDECOMPOSITION_H_ */
