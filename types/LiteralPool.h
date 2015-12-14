/*
 * LiteralsPool.h
 *
 *  Created on: Jul 29, 2015
 *      Author: markus
 */

#ifndef TYPES_LITERALPOOL_H_
#define TYPES_LITERALPOOL_H_

#include "DynamicLiterals.h"
#include "Literal.h"

namespace Dark {

class LiteralPool {
private:
  unsigned int size;
  Literal* cursor;
  Literal* pool;

  void grow();

public:
  typedef Literal* Offset;

  unsigned int initialSize = 100000;

  LiteralPool();
  virtual ~LiteralPool();

  Literal get(int i);
  Literal* resolve(Offset);

  Offset alloc(unsigned int count);
  Offset alloc(Literal literals[], unsigned int count);
  Offset alloc(Literal literals[]);

  void free(Offset reference);
};

} /* namespace Dark */

#endif /* TYPES_LITERALPOOL_H_ */
