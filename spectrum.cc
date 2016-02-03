/*
 * spectrum.cc
 *
 *  Created on: Feb 1, 2016
 *      Author: markus
 */

//#include <climits>
//
//#include <stdlib.h>
//#include <string.h>
//#include <assert.h>
//#include <algorithm>
//
//#include <stdarg.h>
//
//#include <sys/resource.h>

#include <vector>
#include <iostream>
#include <stdio.h>

#include <Eigen/Core>
#include <Eigen/Eigenvalues>

#define VERBOSITY 0
#include "misc/debug.h"
#include "misc/Dimacs.h"

#include "types/ClauseList.h"
#include "types/ClauseIndex.h"
#include "types/PooledLiterals.h"

using namespace Dark;

int main(int argc, char** argv) {
  Dimacs* problem;

  if (argc == 1) {
    fprintf(stderr, "Usage: spectrum [parameters] file\n");
    exit(1);
  }

  char* filename = argv[1];
  int num = 3;
  bool help = false;
  bool verb = false;

  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-n") == 0 && i < argc - 1) {
      num = atoi(argv[++i]);
    }
    else if (strcmp(argv[i], "-v") == 0) {
      verb = true;
    }
    else if (strcmp(argv[i], "-h") == 0) {
      help = true;
    }
    else {
      filename = argv[i];
    }
  }

  if (help) {
    fprintf(stderr, "Usage: spectrum [parameters] file\n");
    fprintf(stderr, "Parameters:\n");
    fprintf(stderr, "-n [number] (no meaning at all)\n");
    exit(0);
  }

  gzFile in = gzopen(filename, "rb");
  if (in == NULL) {
    fprintf(stderr, "Error: Could not open file: %s\n", argc == 1 ? "<stdin>" : argv[1]);
    exit(1);
  }

  std::string sep = "\n----------------------------------------\n";

  problem = new Dimacs(in);
  gzclose(in);
  ClauseList* clauses = problem->getClauses();
  ClauseIndex* index = new ClauseIndex(clauses);
  int nVars = clauses->maxVar()+1;

  Eigen::VectorXi varDeg(nVars);
  for (int i = 0; i < nVars; i++) {
    varDeg[i] = 0;
    if (index->getClauses(mkLit(i, false))) {
      varDeg[i] += index->getClauses(mkLit(i, false))->size();
    }
    if (index->getClauses(mkLit(i, true))) {
      varDeg[i] += index->getClauses(mkLit(i, true))->size();
    }
  }

  Eigen::MatrixXi degrees = varDeg.asDiagonal();
  if (verb) std::cout << degrees << sep;

  Eigen::MatrixXi* adjacency = new Eigen::MatrixXi(nVars, nVars);
  adjacency->setZero();
  for (int i = 0; i < nVars; i++) for (int j = 0; j < 2; j++) {
    ClauseList* list = index->getClauses(mkLit(i, j%2));
    if (list != NULL)
    for (PooledLiterals* lits : *list) {
      for (Literal lit : *lits) {
        if (var(lit) != i) adjacency->coeffRef(i, var(lit)) = 1;
      }
    }
  }
  if (verb) std::cout << *adjacency << sep;

  Eigen::MatrixXi laplacian = degrees - *adjacency;
  if (verb) std::cout << laplacian << sep;

  Eigen::SelfAdjointEigenSolver<Eigen::MatrixXi> es;
  es.compute(laplacian, false);
  cout << "Eigenvalues: " << es.eigenvalues().transpose() << sep;
  //cout << "Eigenvectors: " << es.eigenvectors() << sep;

}

