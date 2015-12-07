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
  unsigned int initialSize = 100000;

  LiteralPool();
  virtual ~LiteralPool();

  Literal get(int i);
  Literal* resolve(Literal*);

  Literal* alloc(unsigned int count);
  Literal* alloc(Literal literals[], unsigned int count);
  Literal* alloc(Literal literals[]);

  void free(Literal* reference);
};

} /* namespace Dark */

#endif /* TYPES_LITERALPOOL_H_ */
