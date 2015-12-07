//============================================================================
// Name        : geninp.cpp
// Author      : Markus Iser
// Version     :
// Copyright   : Public Domain
// Output given CNF plus a comment that identifies the recovered Input Variables
//============================================================================


#include <climits>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <assert.h>
#include <algorithm>

#include <stdarg.h>

#include <sys/resource.h>

#define VERBOSITY 0
#include "misc/debug.h"
#include "misc/Dimacs.h"

#include "tools/GateAnalyzer.h"

#include "types/Literal.h"
#include "types/DynamicLiterals.h"
#include "types/ClauseList.h"

#include "filters/ClauseFilters.h"

using namespace Dark;

Dimacs* problem;
GateAnalyzer* analyzer;

int main(int argc, char** argv) {
  if (argc == 1) {
    fprintf(stderr, "Usage: metrics [parameters] file\n");
    exit(1);
  }

  char* filename = argv[1];
  RootSelectionMethod method = FIRST_CLAUSE;
  EquivalenceDetectionMethod eq_method = PATTERNS;
  int tries = 1;
  bool help = false;

  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-m") == 0 && i < argc - 1) {
      int m = atoi(argv[++i]);
      switch (m) {
      case 1: method = MAX_ID; break;
      case 2: method = MIN_OCCURENCE; break;
      case 3: method = PURE; break;
      default: method = FIRST_CLAUSE;
      }
    }
    else if (strcmp(argv[i], "-e") == 0 && i < argc - 1) {
      int eqd = atoi(argv[++i]);
      switch (eqd) {
      case 1: eq_method = PATTERNS; break;
      case 2: eq_method = SEMANTIC; break;
      default: eq_method = SKIP;
      }
    }
    else if (strcmp(argv[i], "-t") == 0 && i < argc - 1) {
      tries = atoi(argv[++i]);
    }
    else if (strcmp(argv[i], "-h") == 0) {
      help = true;
    }
    else {
      filename = argv[i];
    }
  }

  if (help) {
    fprintf(stderr, "Usage: geninp [parameters] file\n");
    fprintf(stderr, "Parameters:\n");
    fprintf(stderr, "-m [number] (0 - first clause, 1 - max id, 2 - min occurence)\n");
    fprintf(stderr, "-t [number of tries]\n");
    fprintf(stderr, "-h show this help\n");
    fprintf(stderr, "-e (0 - stop on non-monot. gates; 1 - check and/or; 2 - check global up; 3 - check local up)\n");
    exit(0);
  }

  gzFile in = gzopen(filename, "rb");
  if (in == NULL) {
    fprintf(stderr, "Error: Could not open file: %s\n", argc == 1 ? "<stdin>" : argv[1]);
    exit(1);
  }

  problem = new Dimacs(in);
  gzclose(in);
  ClauseList* clauses = problem->getClauses();

  analyzer = new GateAnalyzer(clauses);
  analyzer->analyzeEncoding(method, eq_method, tries);

  vector<int>* inputs = new vector<int>();

  for (int i = 0; i < analyzer->getClauses()->maxVar(); i++) {
    Literal lit1 = mkLit(i, false);
    Literal lit2 = mkLit(i, true);
    if (!analyzer->hasInputs(lit1) && !analyzer->hasInputs(lit2)) {
      inputs->push_back(i+1);
    }
  }

  printf("c inputs ");
  for (vector<int>::iterator it = inputs->begin(); it != inputs->end(); it++) {
    printf("%i ", *it);
  }
  printf("0\n");

  exit(0);
}


