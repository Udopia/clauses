/*
 * PooledLiterals.h
 *
 *  Created on: Jul 29, 2015
 *      Author: markus
 */

#ifndef TYPES_POOLEDLITERALS_H_
#define TYPES_POOLEDLITERALS_H_

#include <vector>
#include <algorithm>
#include <string>
#include <cstdio>

#include "Literal.h"
#include "LiteralPool.h"

namespace Dark {

class PooledLiterals {

private:
  bool mark;

  int max_var;
  int nlits;
  LiteralPool::Offset literals = 0;

  Literal watcher[3] = { litFalse, litFalse, litFalse };

public:
  typedef Literal* iterator;
  typedef const Literal* const_iterator;

  static LiteralPool* pool;

  PooledLiterals();
  PooledLiterals(int count);
  PooledLiterals(Literal lit);
  PooledLiterals(Literal lit1, Literal lit2);
  PooledLiterals(Literal lits[]);

  ~PooledLiterals();

  int maxVar();

  void add(Literal lit);
  void addAll(PooledLiterals* clause);
  void addAll(std::vector<Literal>* clause);
  bool remove(Literal lit);
  Literal removeLast();
  void removeAll(PooledLiterals* clause);
  void sort();

  PooledLiterals* slice(int start);
  PooledLiterals* slice(int start, int end);

  Literal get(int i);
  unsigned int size();

  iterator begin();
  iterator end();
  Literal operator[] (const int i);
  int pos(Literal literal);

  bool contains(Literal literal);
  bool entails(PooledLiterals* clause);
  bool equals(PooledLiterals* clause);

  void print(FILE* out = stdout);
  void println(FILE* out = stdout);
  void printDimacs(FILE* out = stdout);

  std::string* toString();

  PooledLiterals* allBut(Literal lit);

  // from clause
  void setMarked();
  void unsetMarked();
  bool isMarked();

  void inlineNegate();

  bool isBlockedBy(Literal blocking, PooledLiterals* clause);

  // from cube
  PooledLiterals* negate();
  void clear();
  bool isConsistentWith(PooledLiterals* model);

  int cardinality(PooledLiterals* clause);
  bool satisfies(PooledLiterals* clause);
  bool falsifies(PooledLiterals* clause);

};

} /* namespace Dark */

#endif /* TYPES_POOLEDLITERALS_H_ */
