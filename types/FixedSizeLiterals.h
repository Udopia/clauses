/*
 * FixedSizeCube.h
 *
 *  Created on: 10.01.2014
 *      Author: markus
 */

#ifndef FIXEDSIZECUBE_H_
#define FIXEDSIZECUBE_H_

#include "Literal.h"
#include <vector>
#include "DynamicLiterals.h"

namespace Dark {

class FixedSizeLiterals: public DynamicLiterals {
public:
  FixedSizeLiterals(int nVars);
  FixedSizeLiterals(int nVars, DynamicLiterals* list);
  virtual ~FixedSizeLiterals();

  void add(Literal lit);
  void addAll(DynamicLiterals* fsc);

  bool satisfies(Literal literal);
  bool satisfies(DynamicLiterals* clause);
  bool falsifies(DynamicLiterals* clause);
  int cardinality(DynamicLiterals* clause);
  bool lessThanOrEqual(DynamicLiterals* clause, int maxCardinality);

private:
  std::vector<bool>* signs;
  std::vector<bool>* care;
};

} /* namespace Analyzer */
#endif /* CUBE_H_ */
