/*
 * LiteralsPool.cc
 *
 *  Created on: Jul 29, 2015
 *      Author: markus
 */

#include "LiteralPool.h"

#include "Literals.h"
#include "Literal.h"
#include <vector>

namespace Dark {
/**
LiteralPool::LiteralPool(int clausePoolSize, int literalPoolSize) {
  this->clausePoolSize = clausePoolSize;
  this->freeClauses = new std::vector<Literals*>(clausePoolSize);
  this->clausePools = new std::vector<Literals*>();
  newClausePool();
  this->literalPool = (Literal*)malloc(literalPoolSize * sizeof(Literal));
  this->lpSize = literalPoolSize;
  this->lpOffset = 0;
}

LiteralPool::~LiteralPool() {
  for (Literals* pool : clausePools) {
    delete pool;
  }
  delete clausePools;
  delete freeClauses;
}

void LiteralPool::newClausePool() {
  Literals* pool = (Literals*)malloc(clausePoolSize * sizeof(Literals));
  clausePools->push_back(pool);
  for (int i = 0; i < clausePoolSize; i++) {
    freeClauses->push_back(pool+i);
  }
}

void LiteralPool::resizeLiteralPool() {
  lpSize *= 2;
  lpOffset = 0;
  Literal* newPool = (Literal*)malloc(lpSize * sizeof(Literal));
  // copy the literals of all active clauses to the new pool and update their literal-pointer
  for (Literals* pool : clausePools) {
    for (int i = 0; i < clausePoolSize; i++) {
      Literals* clause = pool[i];
      Literal* address = newPool[lpOffset];
      for (int i = 0; i < clause->size(); i++) {
        literalPool[lpOffset++] = clause->literals[i];
      }
      clause->literals = address;
    }
  }
  delete literalPool;
  literalPool = newPool;
}

Literals* LiteralPool::alloc() {
  if (freeClauses->size() == 0) {
    newClausePool();
  }
  return freeClauses->pop_back();
}

void LiteralPool::free(Literals* clause) {
  freeClauses->push_back(clause);
}

Literal* LiteralPool::insertLiterals(Literal literals[], int count) {
  if (lpSize - lpOffset <= count) {
    resizeLiteralPool();
  }
  Literal* address = literalPool[lpOffset];
  for (int i = 0; i < count; i++) {
    literalPool[lpOffset++] = literals[i];
  }
  return address;
}
*/
} /* namespace Dark */
