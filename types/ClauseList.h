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
#include <map>
#include "Literal.h"
#include "../filters/ClauseFilter.h"
#include "DynamicLiterals.h"

using namespace std;

namespace Dark {

class ClauseList {
private:
  typedef std::vector<DynamicLiterals*> InternalClauseList;

protected:
  InternalClauseList* clauses;
  int max_var = 0;

public:
  typedef InternalClauseList::iterator iterator;

  ClauseList();
  ClauseList(ClauseList* clauses);
  ClauseList(std::vector<DynamicLiterals*>* clauses);
  virtual ~ClauseList();

  void freeClauses();

  virtual void add(DynamicLiterals* clause);
  virtual void remove(DynamicLiterals* clause);

  void addAll(ClauseList* list);
  void removeAll(ClauseList* list);

  ClauseList* slice(unsigned int from, unsigned int to);

  DynamicLiterals* get(int i);
  DynamicLiterals* getFirst();
  DynamicLiterals* getLast();
  unsigned int size();

  int maxVar() { return max_var; }
  virtual int newVar() { return ++max_var; }
  void addVarsUntil(Var v) {
    while (max_var < v) {
      newVar();
    }
  }

  int countUsedVariables() {
    vector<bool> used(max_var+1, false);
    for (iterator it = begin(); it != end(); it++) {
      for (DynamicLiterals::iterator lit = (*it)->begin(); lit != (*it)->end(); lit++) {
        used[var(*lit)] = true;
      }
    }
    int result = 0;
    for (int i = 0; i <= max_var; i++) {
      if (used[i]) result++;
    }
    return result;
  }

  iterator begin();
  iterator end();

  void sort(map<DynamicLiterals*, int>* clauseScore);

  int pos(DynamicLiterals* clause);
  DynamicLiterals* find(DynamicLiterals* clause);
  bool contains(DynamicLiterals* clause);

  bool isBlockedBy(Literal lit, DynamicLiterals* clause);
  bool isBlockedBy(Literal lit, ClauseList* list);

  bool matchesFullGatePattern(Literal lit, ClauseList* list);

  DynamicLiterals* getUnionOfLiterals();

  unsigned int minClauseSize();
  unsigned int maxClauseSize();

  ClauseList* getByCriteria(unique_ptr<ClauseFilter> filter);
  ClauseList* removeByCriteria(unique_ptr<ClauseFilter> filter);
  virtual void dumpByCriteria(unique_ptr<ClauseFilter> filter);

  void markAll();
  void unmarkAll();

  void print(FILE* out = stdout);
  void printDimacs(FILE* out = stdout);
};

} /* namespace Analyzer */
#endif /* CLAUSELIST_H_ */
