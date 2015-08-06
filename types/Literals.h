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

class Literals {

private:
  typedef std::vector<Literal> LiteralList;

  bool mark;

  int max_var = 0;

  void Init();

protected:
  LiteralList* literals;

public:
  typedef LiteralList::iterator iterator;

  Literals();
  Literals(Literal lit);
  Literals(Literal lit1, Literal lit2);
  Literals(Literal lit1, Literal lit2, Literal lit3);
  Literals(LiteralList* lits);

  virtual ~Literals();

  int maxVar() { return max_var; }

  void add(Literal lit);
  void addAll(Literals* clause);
  void addAll(std::vector<Literal>* clause);
  bool remove(Literal);
  Literal removeLast();
  void removeAll(Literals* clause);
  void sort();

  Literals* slice(int start);
  Literals* slice(int start, int end);

  iterator begin();
  iterator end();
  Literal getFirst();
  Literal getLast();
  Literal get(int i);
  unsigned int size();

  Literal& operator[] (const int i);
  int pos(Literal literal);

  bool contains(Literal literal);
  bool entails(Literals* clause);
  bool equals(Literals* clause);

  void print(FILE* out = stdout);
  void println(FILE* out = stdout);
  void printDimacs(FILE* out = stdout);

  std::string* toString();

  Literals* allBut(Literal lit);

  // from clause
  void setMarked();
  void unsetMarked();
  bool isMarked();

  void inlineNegate();

  bool isBlockedBy(Literal blocking, Literals* clause);

  // from cube
  Literals* negate();
  void clear();
  bool isConsistentWith(Literals* cube);

  int cardinality(Literals* clause);
  bool satisfies(Literals* clause);
  bool falsifies(Literals* clause);

  ClauseList* checkSatisfied(ClauseList* list);
  Literals* clone();
};

}

#endif /* LITERALS_H_ */
