/*
 * Literals.h
 *
 *  Created on: 28.05.2014
 *      Author: markus
 */

#ifndef LITERALS_H_
#define LITERALS_H_

#include <vector>
#include <algorithm>
#include <string>

#include "Literal.h"

namespace Dark {

class ClauseList;

class DynamicLiterals {

private:
  typedef std::vector<Literal> LiteralList;

  bool mark;

  int max_var = 0;

  void Init();

protected:
  LiteralList* literals;

public:
  typedef LiteralList::iterator iterator;

  DynamicLiterals();
  DynamicLiterals(Literal lit);
  DynamicLiterals(Literal lit1, Literal lit2);
  DynamicLiterals(Literal lit1, Literal lit2, Literal lit3);
  DynamicLiterals(LiteralList* lits);

  virtual ~DynamicLiterals();

  int maxVar() { return max_var; }

  void add(Literal lit);
  void addAll(DynamicLiterals* clause);
  void addAll(std::vector<Literal>* clause);
  bool remove(Literal);
  Literal removeLast();
  void removeAll(DynamicLiterals* clause);
  void sort();

  DynamicLiterals* slice(int start);
  DynamicLiterals* slice(int start, int end);

  iterator begin();
  iterator end();
  Literal getFirst();
  Literal getLast();
  Literal get(int i);
  unsigned int size();

  Literal& operator[] (const int i);
  int pos(Literal literal);

  bool contains(Literal literal);
  bool entails(DynamicLiterals* clause);
  bool equals(DynamicLiterals* clause);

  void print(FILE* out = stdout);
  void println(FILE* out = stdout);
  void printDimacs(FILE* out = stdout);

  std::string* toString();

  DynamicLiterals* allBut(Literal lit);

  // from clause
  void setMarked();
  void unsetMarked();
  bool isMarked();

  void inlineNegate();

  bool isBlockedBy(Literal blocking, DynamicLiterals* clause);

  // from cube
  DynamicLiterals* negate();
  void clear();
  bool isConsistentWith(DynamicLiterals* cube);

  int cardinality(DynamicLiterals* clause);
  bool satisfies(DynamicLiterals* clause);
  bool falsifies(DynamicLiterals* clause);

  ClauseList* checkSatisfied(ClauseList* list);
  DynamicLiterals* clone();
};

}

#endif /* LITERALS_H_ */
