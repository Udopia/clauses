/*
 * PurifiedClauseList.h
 *
 *  Created on: 10.01.2014
 *      Author: markus
 */

#ifndef NORMALIZEDCLAUSELIST_H_
#define NORMALIZEDCLAUSELIST_H_

#include <map>
#include <stdio.h>

#include "ClauseList.h"

namespace Minisat {
  struct Lit;
}

namespace Analyzer {

class Clause;
class Cube;

/**
 * Only keep the satisfied literals (wrt. the given model) of each added clause
 */
class NormalizedClauseList: public Analyzer::ClauseList {
public:
  NormalizedClauseList(Cube* model);
  virtual ~NormalizedClauseList();

  void add(Clause* clause);
  void addAll(ClauseList* list);

  Literal normalize(Literal lit);
  Literal denormalize(Literal lit);

  int getNumberOfVariables() {
    return nVars;
  }

private:
  Cube* model;
  int nVars;

  map<Literal, Literal>* denormalizedLiteral;
};

} /* namespace Analyzer */
#endif /* NORMALIZEDCLAUSELIST_H_ */
