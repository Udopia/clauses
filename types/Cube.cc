/*
 * Cone.cpp
 *
 *  Created on: 10.01.2014
 *      Author: markus
 */

#include "Cube.h"
#include "Literals.h"
#include "ClauseList.h"

#include <algorithm>

namespace Dark {

/**
 * Constructors
 */
Cube::Cube(): Literals() {}

Cube::Cube(Literal lit): Literals(lit) {}

Cube::Cube(Literal lit1, Literal lit2): Literals(lit1, lit2) {}

Cube::Cube(Literal lit1, Literal lit2, Literal lit3): Literals(lit1, lit2, lit3) {}

Cube::Cube(vector<Literal>* lits): Literals(lits) {}

Cube::~Cube() {}


Clause* Cube::negate() {
  Clause* clause = new Clause();
  for (iterator it = begin(); it != end(); ++it) {
    clause->add(~(*it));
  }
  return clause;
}

void Cube::clear() {
  literals->clear();
}

Cube* Cube::clone() {
  Cube* cube = new Cube();
  cube->addAll(this);
  return cube;
}

bool Cube::isConsistentWith(Cube* cube) {
  for (unsigned int i = 0, j = 0; i < this->size() && j < cube->size(); ) {
    Literal lit1 = this->get(i);
    Literal lit2 = cube->get(j);
    if (var(lit1) < var(lit2)) {
      i++;
    } else if (var(lit2) < var(lit1)) {
      j++;
    } else {
      if (lit1 == ~lit2) {
        return false;
      }
      i++;
      j++;
    }
  }
  return true;
}

int Cube::cardinality(Clause* clause) {
  int count = 0;
  for (unsigned int j = 0; j < clause->size(); j++) {
    if (this->contains(clause->get(j))) {
      count++;
    }
  }
  return count;
}

bool Cube::satisfies(Clause* clause) {
  for (unsigned int j = 0; j < clause->size(); j++) {
    if (this->contains(clause->get(j))) {
      return true;
    }
  }
  return false;
}

bool Cube::falsifies(Clause* clause) {
  for (unsigned int j = 0; j < clause->size(); j++) {
    if (!this->contains(~(clause->get(j)))) {
      return false;
    }
  }
  return true;
}

ClauseList* Cube::checkSatisfied(ClauseList* list) {
  ClauseList* notSatisfied = new ClauseList();
  for (unsigned int i = 0; i < list->size(); i++) {
    Clause* clause = list->get(i);
    if (!this->satisfies(clause)) {
      notSatisfied->add(clause);
    }
  }
  return notSatisfied;
}

} /* namespace Analyzer */
