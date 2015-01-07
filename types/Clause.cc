/*
 * Clause.cpp
 *
 *  Created on: 13.12.2013
 *      Author: markus
 */


#include "Clause.h"
#include "stdio.h"

namespace Analyzer {

/**
 * Constructors
 */
Clause::Clause(): Literals() {
  mark = false;
}

Clause::Clause(Literal lit): Literals(lit) {
  mark = false;
}

Clause::Clause(Literal lit1, Literal lit2): Literals(lit1, lit2) {
  mark = false;
}

Clause::Clause(Literal lit1, Literal lit2, Literal lit3): Literals(lit1, lit2, lit3) {
  mark = false;
}

Clause::Clause(vector<Literal>* lits): Literals(lits) {
  mark = false;
}

Clause::~Clause() {}

void Clause::setMarked() {
  mark = true;
}

void Clause::unsetMarked() {
  mark = false;
}

bool Clause::isMarked() {
  return mark;
}

Cube* Clause::negate() {
  Cube* cube = new Cube();
  for (iterator it = begin(); it != end(); ++it) {
    cube->add(~(*it));
  }
  return cube;
}

bool Clause::isBlockedBy(Literal blocking, Clause* clause) {
  for (Clause::iterator it = clause->begin(); it != clause->end(); ++it) {
    Literal& lit = *it;
    if (lit != ~blocking && this->contains(~lit)) {
      return true;
    }
  }
  return false;
}

} /* namespace Analyzer */
