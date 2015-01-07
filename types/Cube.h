/*
 * Cone.h
 *
 *  Created on: 10.01.2014
 *      Author: markus
 */

#ifndef CUBE_H_
#define CUBE_H_

#include "Literals.h"
#include <vector>

namespace Dark {

class Clause;
class ClauseList;

class Cube: public Literals {
public:
  Cube();
  Cube(Literal lit);
  Cube(Literal lit1, Literal lit2);
  Cube(Literal lit1, Literal lit2, Literal lit3);
  Cube(std::vector<Literal>* lits);

  virtual ~Cube();

  Clause* negate();
  void clear();
  bool isConsistentWith(Cube* cube);

  bool satisfies(Clause* clause);

  ClauseList* checkSatisfied(ClauseList* list);
  Cube* clone();
};

} /* namespace Analyzer */
#endif /* CUBE_H_ */
