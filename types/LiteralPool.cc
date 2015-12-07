/*
 * LiteralsPool.cc
 *
 *  Created on: Jul 29, 2015
 *      Author: markus
 */

#include "LiteralPool.h"

#include "Literal.h"
#include <vector>
#include <cstring>
#include "DynamicLiterals.h"

namespace Dark {

LiteralPool::LiteralPool() {
  this->size = initialSize;
  this->pool = (Literal*)malloc(size * sizeof(Literal));
  this->cursor = pool;
}

LiteralPool::~LiteralPool() {
  delete pool;
}

Literal LiteralPool::get(int i) {
  return pool[i];
}

Literal* LiteralPool::resolve(Literal* offset) {
  return (Literal*)(pool + (unsigned int)offset);
}

Literal* LiteralPool::alloc(unsigned int count) {
  size_t offset = cursor - pool;
  if (offset + count + 2 >= size) {
    grow();
  }
  cursor += count;
  *(cursor++) = litFalse;
  *(cursor++) = litFalse;
  return (Literal*)offset;
}

Literal* LiteralPool::alloc(Literal literals[], unsigned int count) {
  size_t offset = cursor - pool; // store offset
  if (offset + count + 2 >= size) { // grow to size
    grow();
  }
  memcpy(cursor, literals, count * sizeof(Literal)); // copy literals
  cursor += count;
  *(cursor++) = litFalse;
  *(cursor++) = litFalse;
  return (Literal*)offset; // return offset
}

Literal* LiteralPool::alloc(Literal literals[]) {
  unsigned int count = 0;
  while (literals[count] != litFalse) { // determine count in zero terminated array
    count++;
  }
  size_t offset = cursor - pool;
  if (offset + count + 2 >= size) {
    grow();
  }
  memcpy(cursor, literals, count * sizeof(Literal));
  cursor += count;
  *(cursor++) = litFalse;
  *(cursor++) = litFalse;
  return (Literal*)offset;
}

void LiteralPool::free(Literal* ref) {
  while (*ref != litFalse) {
    *ref = litFalse;
    ref++;
  }
}

void LiteralPool::grow() {
  size *= 2;
  pool = (Literal*)realloc(pool, size * sizeof(Literal));
}

} /* namespace Dark */
