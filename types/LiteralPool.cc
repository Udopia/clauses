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

Literal* LiteralPool::resolve(Offset offset) {
  return (Literal*)(pool + offset);
}

LiteralPool::Offset LiteralPool::alloc(unsigned int count) {
  Offset offset = cursor - pool;
  if (offset + count + 2 >= size) {
    grow();
  }
  cursor += count;
  *(cursor++) = litFalse;
  *(cursor++) = litFalse;
  return offset;
}

LiteralPool::Offset LiteralPool::alloc(Literal literals[], unsigned int count) {
  Offset offset = cursor - pool; // store offset
  if (offset + count + 2 >= size) { // grow to size
    Literal* buffer = (Literal*)malloc(count * sizeof(Literal));
    memcpy(buffer, literals, count * sizeof(Literal)); // save in buffer due to memory reallocation in grow()
    grow();
    memcpy(cursor, buffer, count * sizeof(Literal)); // copy literals
    delete buffer;
  } else {
    memcpy(cursor, literals, count * sizeof(Literal)); // copy literals
  }
  cursor += count;
  *(cursor++) = litFalse;
  *(cursor++) = litFalse;
  return offset; // return offset
}

LiteralPool::Offset LiteralPool::alloc(Literal literals[]) {
  unsigned int count = 0;
  while (literals[count] != litFalse) { // determine count in zero terminated array
    count++;
  }
  return this->alloc(literals, count);
}

void LiteralPool::free(LiteralPool::Offset ref) {
  Literal* p = this->resolve(ref);
  while (*p != litFalse) {
    *p = litFalse;
    p++;
  }
}

void LiteralPool::grow() {
  size *= 2;
  Offset offset = cursor - pool; // store offset
  pool = (Literal*)realloc(pool, size * sizeof(Literal));
  cursor = pool + offset; // restore cursor
}

} /* namespace Dark */
