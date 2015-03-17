/*
 * Gate.h
 *
 *  Created on: Jan 18, 2015
 *      Author: markus
 */

#ifndef CLAUSES_TOOLS_GATE_H_
#define CLAUSES_TOOLS_GATE_H_

#include <vector>
#include <map>

#include "../types/Literal.h"

using namespace std;

namespace Dark {

class ClauseList;
class Literals;

class Gate {
public:
  Gate(Literal output, ClauseList* forward, ClauseList* backward);
  virtual ~Gate();

  Literal getOutput();
  ClauseList* getForwardClauses();
  ClauseList* getBackwardClauses();

  void addForwardClause(Literals* fwd);

  vector<Literal>* getInputs();
  bool isMonotonous();
  bool isMonotonousIn(Literal literal);

  bool hasNonMonotonousParent();
  void setHasNonMonotonousParent();

  int countAlternatives(Literal input);

private:
  Literal output;
  ClauseList* forward;
  ClauseList* backward;

  bool nonMonotonousParent;

  map<Literal,int>* width;
  vector<Literal>* inputs;

};

}

#endif /* CLAUSES_TOOLS_GATE_H_ */
