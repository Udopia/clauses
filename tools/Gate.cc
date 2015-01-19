/*
 * Gate.cc
 *
 *  Created on: Jan 18, 2015
 *      Author: markus
 */

#include "Gate.h"

#include "../types/ClauseList.h"

namespace Dark {

Gate::Gate(Literal output, ClauseList* forward, ClauseList* backward) {
  this->output = output;
  this->forward = forward;
  this->backward = backward;

  this->inputs = new vector<Literal>();
  for (ClauseList::iterator it = forward->begin(); it != forward->end(); it++) {
    Dark::Clause* clause = *it;
    for (std::vector<Literal>::iterator lit = clause->begin(); lit != clause->end(); ++lit) {
      if (*lit != ~output) inputs->push_back(*lit);
    }
  }
}

Gate::~Gate() {
  delete forward;
  delete backward;
  delete inputs;
}

Literal Gate::getOutput() {
  return output;
}

ClauseList* Gate::getForwardClauses() {
  return forward;
}

ClauseList* Gate::getBackwardClauses() {
  return backward;
}

vector<Literal>* Gate::getInputs() {
  return inputs;
}

bool Gate::isMonotonous() {
  for (vector<Literal>::iterator inp1 = inputs->begin(); inp1 != inputs->end(); inp1++) {
    for (vector<Literal>::iterator inp2 = inp1 + 1; inp2 != inputs->end(); inp2++) {
      if (*inp1 == ~*inp2) return false;
    }
  }
  return true;
}

}
