/*
 * FixedSizeLiterals.cpp
 *
 *  Created on: 10.01.2014
 *      Author: markus
 */

#include "FixedSizeLiterals.h"

#include "Literals.h"

#include <algorithm>

namespace Dark {

/**
 * Constructors
 */
FixedSizeLiterals::FixedSizeLiterals(int nVars) {
  signs = new std::vector<bool>(nVars, false);
  care = new std::vector<bool>(nVars, false);
}

FixedSizeLiterals::FixedSizeLiterals(int nVars, Literals* cube) {
  signs = new std::vector<bool>(nVars, false);
  care = new std::vector<bool>(nVars, false);
  for (Literals::iterator it = cube->begin(); it != cube->end(); it++) {
    this->add(*it);
  }
}

FixedSizeLiterals::~FixedSizeLiterals() {
  delete signs;
  delete care;
}

void FixedSizeLiterals::add(Literal lit) {
  Literals::add(lit);
  (*signs)[var(lit)] = sign(lit);
  (*care)[var(lit)] = true;
}

void FixedSizeLiterals::addAll(Literals* fsc) {
  for (std::vector<Literal>::iterator it = fsc->begin(); it != fsc->end(); it++) {
    add(*it);
  }
}

bool FixedSizeLiterals::satisfies(Literal lit) {
    return (*care)[var(lit)] && (*signs)[var(lit)] == sign(lit);
}

bool FixedSizeLiterals::satisfies(Literals* clause) {
  for (unsigned int j = 0; j < clause->size(); j++) {
    Literal lit = clause->get(j);
    if (this->satisfies(lit)) {
      return true;
    }
  }
  return false;
}

bool FixedSizeLiterals::falsifies(Literals* clause) {
  for (unsigned int j = 0; j < clause->size(); j++) {
    Literal lit = clause->get(j);
    if (!(*care)[var(lit)] || (*signs)[var(lit)] == sign(lit)) {
      return false;
    }
  }
  return true;
}

int FixedSizeLiterals::cardinality(Literals* clause) {
  int card = 0;
  for (unsigned int j = 0; j < clause->size(); j++) {
    Literal lit = clause->get(j);
    if ((*signs)[var(lit)] == sign(lit)) {
      card++;
    }
  }
  return card;
}

bool FixedSizeLiterals::lessThanOrEqual(Literals* clause, int maxCardinality) {
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
