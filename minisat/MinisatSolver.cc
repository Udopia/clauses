/*
 * MinisatSolver.cc
 *
 *  Created on: Feb 18, 2015
 *      Author: markus
 */

#include "MinisatSolver.h"
#include "../types/ClauseList.h"
#include "../types/Literals.h"
#include "../types/Literal.h"

using namespace Dark;

MinisatSolver::MinisatSolver(ClauseList* list) {
  this->solver = new Minisat::Solver();
  for (int i = 0; i < list->maxVar(); i++) {
    this->solver->newVar();
  }
  for (auto clause : *list) {
    toTmpVec(clause);
    this->solver->addClause_(tmpVec);
  }
}

MinisatSolver::~MinisatSolver() {
  delete solver;
}

void MinisatSolver::toTmpVec(Literals* lits) {
  tmpVec.clear();
  for (Literal lit : *lits) {
    tmpVec.push(static_cast<Minisat::Lit>(lit));
  }
}

bool MinisatSolver::isUPConsistent(Literals* cube) {
  toTmpVec(cube);
  Minisat::vec<Minisat::Lit> tmp_out;
  return solver->implies(tmpVec, tmp_out);
}
