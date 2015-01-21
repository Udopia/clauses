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
#include "Clause.h"
#include "Literal.h"
#include "../filters/ClauseFilter.h"

using namespace std;

namespace Dark {

class ClauseList {
private:
  typedef std::vector<Clause*> InternalClauseList;

protected:
  InternalClauseList* clauses;

public:
  typedef InternalClauseList::iterator iterator;

  ClauseList();
  virtual ~ClauseList();

  void freeClauses();

  virtual void add(Clause* clause);
  void addAll(ClauseList* list);
  Clause* get(int i);
  Clause* getFirst();
  Clause* getLast();
  unsigned int size();


  iterator begin();
  iterator end();

  Clause* find(Clause* clause);
  bool contains(Clause* clause);

  bool isBlockedBy(Literal lit, Clause* clause);
  bool isBlockedBy(Literal lit, ClauseList* list);

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
