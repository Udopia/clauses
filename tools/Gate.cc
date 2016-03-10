/*
 * Gate.cc
 *
 *  Created on: Jan 18, 2015
 *      Author: markus
 */

#include "Gate.h"

#include "../types/ClauseList.h"

namespace Dark {

Gate::Gate(Literal output, ClauseList* fwd, ClauseList* bwd) {
  this->output = output;
  this->forward = new ClauseList(fwd);
  this->backward = new ClauseList(bwd);
  this->nonMonotonousParent = false;

  this->inputs = new vector<Literal>();
  for (ClauseList::iterator clause = forward->begin(); clause != forward->end(); clause++) {
    for (PooledLiterals::iterator lit = (*clause)->begin(); *lit != litFalse; ++lit) {
      if (*lit != ~output) {
        inputs->push_back(*lit);
      }
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

void Gate::addForwardClause(PooledLiterals* fwd) {
  forward->add(fwd);
  for (PooledLiterals::iterator lit = fwd->begin(); *lit != litFalse; ++lit) {
    if (*lit != ~output) {
      inputs->push_back(*lit);
    }
  }
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
  for (std::vector<Literal>::iterator inp1 = inputs->begin(); inp1 != inputs->end(); inp1++) {
    for (std::vector<Literal>::iterator inp2 = inp1 + 1; inp2 != inputs->end(); inp2++) {
      if (*inp1 == ~*inp2) return false;
    }
  }
  return true;
}

bool Gate::isMonotonousIn(Literal literal) {
  for (std::vector<Literal>::iterator inp = inputs->begin(); inp != inputs->end(); inp++) {
    if (*inp == ~literal) return false;
  }
  return true;
}

bool Gate::hasNonMonotonousParent() {
  return nonMonotonousParent;
}

void Gate::setHasNonMonotonousParent() {
  this->nonMonotonousParent = true;
}

}
