/*
 * LiteralsPool.h
 *
 *  Created on: Jul 29, 2015
 *      Author: markus
 */

#ifndef TYPES_LITERALPOOL_H_
#define TYPES_LITERALPOOL_H_

#include "Literal.h"
#include "Literals.h"
#include <vector>

namespace Dark {

class LiteralPool {

private:
  /**
   * Keep a list of fixed size pools and
   * keep track of the freed clauses
   */
  std::vector<Literals*>* clausePools;
  int clausePoolSize;
  std::vector<Literals*>* freeClauses;

  /**
   * Track the literals in a separate pool that may fragment
   * Keep track of the size and the cursor position
   */
  Literal* literalPool;
  int lpOffset;
  int lpSize;

  void newClausePool();
  void resizeLiteralPool();

public:
  LiteralPool(int clausePoolSize = 100000, int literalPoolSize = 500000);
  virtual ~LiteralPool();

  Literals* alloc();
  void free(Literals*);

  Literal* insertLiterals(Literal literals[], int count);
};

} /* namespace Dark */

#endif /* TYPES_LITERALPOOL_H_ */
