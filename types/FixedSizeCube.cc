/*
 * FixedSizeCube.cpp
 *
 *  Created on: 10.01.2014
 *      Author: markus
 */

#include "FixedSizeCube.h"

#include "ClauseList.h"

#include <algorithm>

namespace Analyzer {

/**
 * Constructors
 */
FixedSizeCube::FixedSizeCube(int nVars) {
  signs = new std::vector<bool>(nVars, false);
  care = new std::vector<bool>(nVars, false);
}

FixedSizeCube::FixedSizeCube(int nVars, Cube* cube) {
  signs = new std::vector<bool>(nVars, false);
  care = new std::vector<bool>(nVars, false);
  for (Cube::iterator it = cube->begin(); it != cube->end(); it++) {
    this->add(*it);
  }
}

FixedSizeCube::~FixedSizeCube() {
  delete signs;
  delete care;
}

void FixedSizeCube::add(Literal lit) {
  Literals::add(lit);
  (*signs)[var(lit)] = sign(lit);
  (*care)[var(lit)] = true;
}

bool FixedSizeCube::satisfies(Literal lit) {
    return (*care)[var(lit)] && (*signs)[var(lit)] == sign(lit);
}

bool FixedSizeCube::satisfies(Clause* clause) {
  for (unsigned int j = 0; j < clause->size(); j++) {
    Literal lit = clause->get(j);
    if (this->satisfies(lit)) {
      return true;
    }
  }
  return false;
}

bool FixedSizeCube::falsifies(Clause* clause) {
  for (unsigned int j = 0; j < clause->size(); j++) {
    Literal lit = clause->get(j);
    if (!(*care)[var(lit)] || (*signs)[var(lit)] == sign(lit)) {
      return false;
    }
  }
  return true;
}

int FixedSizeCube::cardinality(Clause* clause) {
  int card = 0;
  for (unsigned int j = 0; j < clause->size(); j++) {
    Literal lit = clause->get(j);
    if ((*signs)[var(lit)] == sign(lit)) {
      card++;
    }
  }
  return card;
}

bool FixedSizeCube::lessThanOrEqual(Clause* clause, int maxCardinality) {
  int card = 0;
  for (unsigned int j = 0; j < clause->size(); j++) {
    Literal lit = clause->get(j);
    if ((*signs)[var(lit)] == sign(lit)) {
      card++;
      if (card > maxCardinality) {
        return false;
      }
    }
  }
  return true;
}

} /* namespace Analyzer */
