/*
 * PooledLiterals.h
 *
 *  Created on: Jul 29, 2015
 *      Author: markus
 */

#ifndef TYPES_POOLEDLITERALS_H_
#define TYPES_POOLEDLITERALS_H_

#include "Literal.h"

namespace Dark {

class PooledLiterals {

public:
  PooledLiterals();
  virtual ~PooledLiterals();

private:
  Literal first;
  Literal second;
  Literal third;

};

} /* namespace Dark */

#endif /* TYPES_POOLEDLITERALS_H_ */
