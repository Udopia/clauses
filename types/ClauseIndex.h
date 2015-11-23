/*
 * ClauseIndex.h
 *
<<<<<<< HEAD
 *  Created on: Oct 31, 2015
=======
 *  Created on: Nov 5, 2015
>>>>>>> c209260b962b7b645e971e5ce384b5bc2f89df3a
 *      Author: markus
 */

#ifndef TYPES_CLAUSEINDEX_H_
#define TYPES_CLAUSEINDEX_H_

#include <map>

#include "Literal.h"

namespace Dark {

class Literals;
class ClauseList;

class ClauseIndex {
private:
  std::map<Literal, Dark::ClauseList*>* clauseMap;
  int nVars;

  void createVars(Var v);

public:
  ClauseIndex();
  ClauseIndex(ClauseList* clauses);
  virtual ~ClauseIndex();

  void add(Literals* clause);
  void addAll(ClauseList* clause);
  void remove(Literals* clause);
  void removeAll(ClauseList* clause);

  void augment(Literals* clause, Literal lit);
  void augment(ClauseList* clauses, Literal lit);

  ClauseList* getClauses(Literal literal);

  int countOccurence(Literal literal);
};

}
#endif /* TYPES_CLAUSEINDEX_H_ */
