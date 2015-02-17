/*
 * Clause.h
 *
 *  Created on: 13.12.2013
 *      Author: markus
 */

#ifndef CLAUSE_H_
#define CLAUSE_H_

#include <vector>

#include "Literals.h"
#include "Cube.h"

using namespace std;

namespace Dark {

class Clause: public Literals {
private:
  bool mark;

public:
  Clause();
  Clause(Literal lit);
  Clause(Literal lit1, Literal lit2);
  Clause(Literal lit1, Literal lit2, Literal lit3);
  Clause(vector<Literal>* lits);

  virtual ~Clause();

  void setMarked();
  void unsetMarked();
  bool isMarked();

  Cube* negate();
  void inlineNegate();

  bool isBlockedBy(Literal blocking, Clause* clause);

  Clause* allBut(Literal exclude);

};

} /* namespace Analyzer */
#endif /* CLAUSE_H_ */
