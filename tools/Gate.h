/*
 * Gate.h
 *
 *  Created on: Jan 18, 2015
 *      Author: markus
 */

#ifndef CLAUSES_TOOLS_GATE_H_
#define CLAUSES_TOOLS_GATE_H_

#include <vector>

#include "../types/Literal.h"

using namespace std;

namespace Dark {

class ClauseList;

class Gate {
public:
  Gate(Literal output, ClauseList* forward, ClauseList* backward);
  virtual ~Gate();

  Literal getOutput();
  ClauseList* getForwardClauses();
  ClauseList* getBackwardClauses();

  vector<Literal>* getInputs();
  bool isMonotonous();

private:
  Literal output;
  ClauseList* forward;
  ClauseList* backward;

  vector<Literal>* inputs;

};

}

#endif /* CLAUSES_TOOLS_GATE_H_ */
