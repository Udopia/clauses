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

namespace Dark {

class Literals {

private:
  typedef std::vector<Literal> LiteralList;

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

  void add(Literal lit);
  void addAll(Literals* clause);
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

  bool contains(Literal literal);
  bool entails(Literals* clause);
  bool equals(Literals* clause);

  void print(FILE* out = stdout);
  void println(FILE* out = stdout);

  std::string* toString();

  Literals* allBut(Literal lit);

};

}

#endif /* LITERALS_H_ */
