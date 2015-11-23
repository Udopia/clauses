//============================================================================
// Name        : cnf2aig.cpp
// Author      : Markus Iser
// Version     :
// Copyright   : Public Domain
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
#include "tools/Gate.h"
#include "tools/BlockedClauseDecomposition.h"

#include "types/Literal.h"
#include "types/Literals.h"
#include "types/ClauseList.h"
#include "types/ClauseIndex.h"
#include "filters/ClauseFilters.h"

using namespace Dark;

Dimacs* problem;
GateAnalyzer* analyzer;

vector<int>* minDepth;
vector<int>* minDepth2;

void setDepths(Literal root) {
  int depth = 0;

  vector<Literal>* literals = new vector<Literal>();
  vector<Literal>* next = new vector<Literal>();
  next->push_back(root);

  while (next->size() > 0) {
    literals->insert(literals->end(), next->begin(), next->end());
    next->clear();

    for (vector<Literal>::iterator it = literals->begin(); it != literals->end(); it++) {
      Literal literal = *it;

      if ((*minDepth)[var(literal)] > depth) {
        (*minDepth)[var(literal)] = depth;

        Gate* gate = analyzer->getGate(literal);
        if (gate == NULL) continue;

        vector<Literal>* inputs = gate->getInputs();
        next->insert(next->end(), inputs->begin(), inputs->end());
      }
    }

    depth++;
  }

  //delete literals;
}

void setDepths2(Literal root) {
  int depth = 0;

  vector<Literal>* literals = new vector<Literal>();
  vector<Literal>* next = new vector<Literal>();
  next->insert(next->end(), analyzer->getGate(root)->getInputs()->begin(),
      analyzer->getGate(root)->getInputs()->end());

  while (next->size() > 0) {
    literals->insert(literals->end(), next->begin(), next->end());
    next->clear();

    for (vector<Literal>::iterator it = literals->begin(); it != literals->end(); it++) {
      Literal literal = *it;

      if ((*minDepth2)[var(literal)] > depth) {
        (*minDepth2)[var(literal)] = depth;

        Gate* gate = analyzer->getGate(literal);
        if (gate == NULL) continue;

        vector<Literal>* inputs = gate->getInputs();
        next->insert(next->end(), inputs->begin(), inputs->end());
      }
    }

    depth++;
  }

  //delete literals;
}

double cpuTime() {
  struct rusage ru;
  getrusage(RUSAGE_SELF, &ru);
  return (double)ru.ru_utime.tv_sec + (double)ru.ru_utime.tv_usec / 1000000;
}

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
  bool purity = false;
  bool bcd = false;

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
    else if (strcmp(argv[i], "-b") == 0) {
      bcd = true;
    }
    else if (strcmp(argv[i], "-p") == 0) {
      purity = true;
    }
    else {
      filename = argv[i];
    }
  }

  if (help) {
    fprintf(stderr, "Usage: metrics [parameters] file\n");
    fprintf(stderr, "Parameters:\n");
    fprintf(stderr, "-m [number] (0 - first clause, 1 - max id, 2 - min occurence)\n");
    fprintf(stderr, "-t [number of tries]\n");
    fprintf(stderr, "-h show this help\n");
    fprintf(stderr, "-p show number of pure gates\n");
    fprintf(stderr, "-b bcd before analysis\n");
    fprintf(stderr, "-e (0 - stop on non-monot. gates; 1 - check and/or; 2 - check global up; 3 - check local up)\n");
    fprintf(stderr, "filename, nVars, nClauses, nGates, maxDepth1, maxDepth2, maxWidth1, maxWidth2, maxWidth3, medWidth, endTime - startTime\n");
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
  if (bcd) {
    BlockedClauseDecomposition bcDec(clauses);
    bcDec.decompose();
    bcDec.postprocess();
    bcDec.shiftSmallByUnit();
  }
  ClauseIndex* index = new ClauseIndex(clauses);

  minDepth = new vector<int>(problem->getDeclNVars(), INT_MAX);
  minDepth2 = new vector<int>(problem->getDeclNVars(), INT_MAX);
  int nVars = problem->getDeclNVars();

  delete problem;

  if (clauses->size() == 0) {
    printf("no clauses\n");
    return 1;
  }

  double startTime = cpuTime();
  analyzer = new GateAnalyzer(clauses);
  analyzer->analyzeEncoding(method, eq_method, tries);
  double endTime = cpuTime();

  int usedVars = clauses->countUsedVariables();
  int nClauses = clauses->size();
  vector<int>* nClausesInGate = new vector<int>();
  int nGates = 0;

  int nSimpleGates = 0;
  int nUsedGates = 0;

  for (int i = 0; i < nVars+1; i++) {
    Gate* gate = analyzer->getGate(mkLit(i, false));
    if (gate == NULL) {
      gate = analyzer->getGate(mkLit(i, true));
    }
    if (gate != NULL) {
      Literal output = gate->getOutput();
      nGates++;
      if (gate->getBackwardClauses()->size() == 0) {
        nSimpleGates++;
      }
      if (analyzer->getParents(output) != NULL && analyzer->getParents(output)->size() > 1) {
        nUsedGates++;
      }
      nClausesInGate->push_back(gate->getForwardClauses()->size() + gate->getBackwardClauses()->size());
    }
  }
  sort(nClausesInGate->begin(), nClausesInGate->end());
  //int min = (*nClausesInGate)[0];
  int medWidth = (*nClausesInGate)[nClausesInGate->size() / 2];
  int maxWidth3 = (*nClausesInGate)[nClausesInGate->size() - 3];
  int maxWidth2 = (*nClausesInGate)[nClausesInGate->size() - 2];
  int maxWidth1 = (*nClausesInGate)[nClausesInGate->size() - 1];

  setDepths(analyzer->getRoot());
  setDepths2(analyzer->getRoot());

  int maxDepth1 = 0;
  for (vector<int>::iterator it = minDepth->begin(); it != minDepth->end(); it++) {
    if (*it != INT_MAX) {
      if (*it > maxDepth1) maxDepth1 = *it;
    }
  }
  int maxDepth2 = 0;
  for (vector<int>::iterator it = minDepth2->begin(); it != minDepth2->end(); it++) {
    if (*it != INT_MAX) {
      if (*it > maxDepth2) maxDepth2 = *it;
    }
  }

  if (purity) {
    fprintf(stdout, "%s,%i,%i,%i\n", filename, nGates, nSimpleGates, nUsedGates);
  } else {
    fprintf(stdout, "%s,%i,%i,%i,%i,%i,%i,%i,%i,%i,%.2f\n",
        filename, usedVars, nClauses, nGates, maxDepth1, maxDepth2, maxWidth1, maxWidth2, maxWidth3, medWidth, endTime - startTime);
  }

  exit(0);
}


