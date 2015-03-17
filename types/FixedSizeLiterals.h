/*
 * FixedSizeCube.h
 *
 *  Created on: 10.01.2014
 *      Author: markus
 */

#ifndef FIXEDSIZECUBE_H_
#define FIXEDSIZECUBE_H_

#include "Literal.h"
#include "Literals.h"
#include <vector>

namespace Dark {

class FixedSizeLiterals: public Literals {
public:
  FixedSizeLiterals(int nVars);
  FixedSizeLiterals(int nVars, Literals* list);
  virtual ~FixedSizeLiterals();

  void add(Literal lit);
  void addAll(Literals* fsc);

  bool satisfies(Literal literal);
  bool satisfies(Literals* clause);
  bool falsifies(Literals* clause);
  int cardinality(Literals* clause);
  bool lessThanOrEqual(Literals* clause, int maxCardinality);

private:
  std::vector<bool>* signs;
  std::vector<bool>* care;
};

} /* namespace Analyzer */
#endif /* CUBE_H_ */
