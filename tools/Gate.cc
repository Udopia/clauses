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
  this->width = new map<Literal,int>();

  //printf("New Gate with output %s%i\n", sign(output)?"-":"", var(output)+1);
  //forward->print();
  //printf("\n");

  this->inputs = new vector<Literal>();
  for (ClauseList::iterator it = forward->begin(); it != forward->end(); it++) {
    Dark::Clause* clause = *it;
    for (std::vector<Literal>::iterator lit = clause->begin(); lit != clause->end(); ++lit) {
      if (*lit != ~output) {
        inputs->push_back(*lit);
        if (width->count(*lit) == 0) {
          (*width)[*lit] = clause->size();
        } else if ((*width)[*lit] < (int)clause->size()) {
          (*width)[*lit] = clause->size();
        }
      }
    }
  }
}

Gate::~Gate() {
  delete forward;
  delete backward;
  delete inputs;
  delete width;
}

Literal Gate::getOutput() {
  return output;
}

void Gate::addForwardClause(Clause* fwd) {
  forward->add(fwd);
  for (std::vector<Literal>::iterator lit = fwd->begin(); lit != fwd->end(); ++lit) {
    if (*lit != ~output) {
      inputs->push_back(*lit);
      if (width->count(*lit) == 0) {
        (*width)[*lit] = fwd->size();
      } else if ((*width)[*lit] < (int)fwd->size()) {
        (*width)[*lit] = fwd->size();
      }
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
  for (vector<Literal>::iterator inp1 = inputs->begin(); inp1 != inputs->end(); inp1++) {
    for (vector<Literal>::iterator inp2 = inp1 + 1; inp2 != inputs->end(); inp2++) {
      if (*inp1 == ~*inp2) return false;
    }
  }
  return true;
}

bool Gate::isMonotonousIn(Literal literal) {
  for (vector<Literal>::iterator inp = inputs->begin(); inp != inputs->end(); inp++) {
    if (*inp == ~literal) return false;
  }
  return true;
}

int Gate::countAlternatives(Literal input) {
  return (*width)[input];
}

}
