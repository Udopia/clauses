/*
 * FixedSizeCube.h
 *
 *  Created on: 10.01.2014
 *      Author: markus
 */

#ifndef FIXEDSIZECUBE_H_
#define FIXEDSIZECUBE_H_

#include "Cube.h"
#include <vector>

namespace Dark {

class Clause;
class ClauseList;

class FixedSizeCube: public Cube {
public:
  FixedSizeCube(int nVars);
  FixedSizeCube(int nVars, Cube* cube);
  virtual ~FixedSizeCube();

  void add(Literal lit);

  bool satisfies(Literal literal);
  bool satisfies(Clause* clause);
  bool falsifies(Clause* clause);
  int cardinality(Clause* clause);
  bool lessThanOrEqual(Clause* clause, int maxCardinality);

private:
  std::vector<bool>* signs;
  std::vector<bool>* care;
};

} /* namespace Analyzer */
#endif /* CUBE_H_ */
