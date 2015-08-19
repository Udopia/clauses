/*
 * MinisatSolver.h
 *
 *  Created on: Feb 18, 2015
 *      Author: markus
 */

#ifndef CLAUSES_TOOLS_MINISATSOLVER_H_
#define CLAUSES_TOOLS_MINISATSOLVER_H_

#include "solver/minisat/core/Solver.h"
#include "solver/minisat/core/SolverTypes.h"
#include "solver/minisat/mtl/Vec.h"

namespace Dark {
  class ClauseList;
  class Literals;

  class MinisatSolver {
  private:
    Minisat::Solver* solver;
    Minisat::vec<Minisat::Lit> tmpVec;

    void toTmpVec(Literals* lits);

  public:
    MinisatSolver(ClauseList* list);
    virtual ~MinisatSolver();

    bool isUPConsistent(Literals* cube);
  };
}
#endif /* CLAUSES_TOOLS_MINISATSOLVER_H_ */
