/*
 * ClauseList.h
 *
 *  Created on: 13.12.2013
 *      Author: markus
 */

#ifndef CLAUSELIST_H_
#define CLAUSELIST_H_

#include <vector>
#include <memory>
#include "Literals.h"
#include "Literal.h"
#include "../filters/ClauseFilter.h"

using namespace std;

namespace Dark {

class ClauseList {
private:
  typedef std::vector<Literals*> InternalClauseList;
  int max_var = 0;

protected:
  InternalClauseList* clauses;

public:
  typedef InternalClauseList::iterator iterator;

  ClauseList();
  virtual ~ClauseList();

  void freeClauses();

  virtual void add(Literals* clause);
  void addAll(ClauseList* list);
  Literals* get(int i);
  Literals* getFirst();
  Literals* getLast();
  unsigned int size();

  int maxVar() { return max_var; }

  iterator begin();
  iterator end();

  Literals* find(Literals* clause);
  bool contains(Literals* clause);

  bool isBlockedBy(Literal lit, Literals* clause);
  bool isBlockedBy(Literal lit, ClauseList* list);

  bool definesEquivalence(Literal lit, ClauseList* list);

  Literals* getUnionOfLiterals();

  unsigned int minClauseSize();
  unsigned int maxClauseSize();

  ClauseList* getByCriteria(unique_ptr<ClauseFilter> filter);
  ClauseList* removeByCriteria(unique_ptr<ClauseFilter> filter);
  void dumpByCriteria(unique_ptr<ClauseFilter> filter);

  void markAll();
  void unmarkAll();

  void print(FILE* out = stdout);
  void printDimacs(FILE* out = stdout);
};

} /* namespace Analyzer */
#endif /* CLAUSELIST_H_ */
