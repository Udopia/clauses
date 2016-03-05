//============================================================================
// Title       : Justine
// Quote       : "Why do you complain of your fate when you could so easily change it?"
// Author      : Markus Iser
// Version     : 1
// Copyright   : Public Domain
//============================================================================


#include <iostream>

#include <climits>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <set>
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
#include "types/DynamicLiterals.h"
#include "types/ClauseList.h"
#include "types/ClauseIndex.h"
#include "types/DynamicLiterals.h"
#include "filters/ClauseFilters.h"

using namespace Dark;

GateAnalyzer* analyzer;

vector<int>* width;
vector<int>* inputs;
map<int, set<int>*>* inputVariables;
map<PooledLiterals*, DynamicLiterals*>* litsToAdd;

int widthAbsLB = 0;
int leavesAbsLB = 0;
double widthRelLB = 0.0;
double leavesRelLB = 0.0;

double maxWidth;
double maxLeaves;

void calculateAlternatives(Literal root) {
  Gate* gate = analyzer->getGate(root);
  if (gate == NULL) return;

  int parent_width = (*width)[var(root)];
  for (PooledLiterals* clause : *gate->getForwardClauses()) {
    for (Literal lit : *clause) if (var(lit) != var(root)) {
      const int old_width = (*width)[var(lit)];
      const int new_width = parent_width * (clause->size()-1);
      (*width)[var(lit)] = old_width > new_width ? old_width : new_width;
      calculateAlternatives(lit);
    }
  }
}

void calculateInputVariables(Literal root) {
  (*inputVariables)[var(root)] = new set<int>();

  Gate* gate = analyzer->getGate(root);
  if (gate != NULL) {
    for (PooledLiterals* clause : *gate->getForwardClauses()) {
      for (Literal lit : *clause) if (var(lit) != var(root)) {
        calculateInputVariables(lit);
        set<int>* leafs = (*inputVariables)[var(lit)];
        (*inputVariables)[var(root)]->insert(leafs->begin(), leafs->end());
      }
    }
  }
  else {
    (*inputVariables)[var(root)]->insert(var(root));
  }

  (*inputs)[var(root)] = (*inputVariables)[var(root)]->size();
}

set<PooledLiterals*>* getInputClauses(Literal root) {
  set<PooledLiterals*>* result = new set<PooledLiterals*>();
  Gate* gate = analyzer->getGate(root);
  if (gate != NULL) for (PooledLiterals* clause : *gate->getForwardClauses()) {
    for (Literal lit : *clause) if (var(lit) != var(root)) {
      set<PooledLiterals*>* more;
      if (analyzer->getGate(lit) != NULL) {
        more = getInputClauses(lit);
      } else {
        more = new set<PooledLiterals*>();
        more->insert(clause);
      }
      result->insert(more->begin(), more->end());
    }
  }
  return result;
}

bool matchesBounds(Literal root) {
  return (*width)[var(root)] >= widthAbsLB && (*inputs)[var(root)] >= leavesAbsLB
      && (*width)[var(root)] / maxWidth >= widthRelLB && (*inputs)[var(root)] / maxLeaves >= leavesRelLB;
}

void addLiteralToItsLeaves(Literal root) {
  set<PooledLiterals*>* leaves = getInputClauses(root);
  for (PooledLiterals* leaf : *leaves) {
    if (litsToAdd->count(leaf) == 0) {
      (*litsToAdd)[leaf] = new DynamicLiterals(root);
    } else if (!(*litsToAdd)[leaf]->contains(root)) {
      (*litsToAdd)[leaf]->add(root);
    }
  }
}

void calculateLiteralsToAdd(Literal root) {
  if (matchesBounds(root)) {
    addLiteralToItsLeaves(root);
    return;
  }

  Gate* gate = analyzer->getGate(root);
  if (gate == NULL) return;
  if (gate->hasNonMonotonousParent()) return;

  for (PooledLiterals* clause : *gate->getForwardClauses()) {
    for (Literal lit : *clause) if (var(lit) != var(root)) {
      calculateLiteralsToAdd(lit);
    }
  }
}


double cpuTime() {
  struct rusage ru;
  getrusage(RUSAGE_SELF, &ru);
  return (double)ru.ru_utime.tv_sec + (double)ru.ru_utime.tv_usec / 1000000;
}

int main(int argc, char** argv) {
  if (argc == 1) {
    fprintf(stderr, "Usage: preprocess [parameters] file\n");
    exit(1);
  }

  char* filename = argv[1];
  RootSelectionMethod method = MIN_OCCURENCE;
  EquivalenceDetectionMethod eq_method = PATTERNS;
  int tries = 3;
  bool help = false;
  bool bcd = false;
  double minRate = 0.0;

  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-m") == 0 && i < argc - 1) {
      int m = atoi(argv[++i]);
      switch (m) {
      case 2: method = MIN_OCCURENCE; break;
      case 3: method = PURITY; break;
      default: method = MIN_OCCURENCE;
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
    else if (strcmp(argv[i], "-minrate") == 0 && i < argc - 1) {
      minRate = atof(argv[++i]);
    }
    else if (strcmp(argv[i], "-wabs") == 0 && i < argc - 1) {
      widthAbsLB = atoi(argv[++i]);
    }
    else if (strcmp(argv[i], "-labs") == 0 && i < argc - 1) {
      leavesAbsLB = atoi(argv[++i]);
    }
    else if (strcmp(argv[i], "-wrel") == 0 && i < argc - 1) {
      widthRelLB = atof(argv[++i]);
    }
    else if (strcmp(argv[i], "-lrel") == 0 && i < argc - 1) {
      leavesRelLB = atof(argv[++i]);
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
    else {
      filename = argv[i];
    }
  }

  if (help) {
    fprintf(stderr, "Usage: preprocess [parameters] file\n");
    fprintf(stderr, "Parameters:\n");
    fprintf(stderr, "-m [number] (0 - first clause, 1 - max id, 2 - min occurence)\n");
    fprintf(stderr, "-t [number of tries]\n");
    fprintf(stderr, "-h show this help\n");
    fprintf(stderr, "-b bcd before analysis\n");
    exit(0);
  }

  gzFile in = gzopen(filename, "rb");
  if (in == NULL) {
    fprintf(stderr, "Error: Could not open file: %s\n", argc == 1 ? "<stdin>" : argv[1]);
    exit(1);
  }

  Dimacs* problem = new Dimacs(in);
  gzclose(in);
  ClauseList* clauses = problem->getClauses();
  if (bcd) {
    BlockedClauseDecomposition bcDec(clauses);
    bcDec.decomposeClauses();
  }

  if (clauses->size() == 0) {
    cerr << "no clauses" << endl;
    return 1;
  }

  analyzer = new GateAnalyzer(clauses, method, eq_method);
  analyzer->analyzeEncoding(tries);

  width = new vector<int>(clauses->maxVar()+1, 1);
  calculateAlternatives(analyzer->getRoot());

  inputs = new vector<int>(clauses->maxVar()+1, 0);
  inputVariables = new map<int, set<int>*>();
  calculateInputVariables(analyzer->getRoot());

  maxWidth = *std::max_element(width->begin(), width->end());
  maxLeaves = *std::max_element(inputs->begin(), inputs->end());

  double recognitionRate = ((double)clauses->maxVar() - maxLeaves) / (double)clauses->maxVar();
  if (recognitionRate < minRate) {
    cerr << "Recognition Rate " << recognitionRate << " does not meet Minimum Rate " << minRate << endl;
    //clauses->printDimacs();
    return 0;
  }
  else {
    cerr << "Recognition Rate " << recognitionRate << " meets Minimum Rate " << minRate << endl;
  }

  litsToAdd = new map<PooledLiterals*, DynamicLiterals*>();
  calculateLiteralsToAdd(analyzer->getRoot());

  delete analyzer;

//  clauses->print(stderr);
//  cerr << endl;
//
//  for (int i : *width) {
//    cout << i << ", ";
//  }
//  cout << endl;
//
//  for (int i : *inputs) {
//    cout << i << ", ";
//  }
//  cout << endl;
//
//  for(map<int, set<int>*>::iterator iter = inputVariables->begin(); iter != inputVariables->end(); ++iter) {
//    cout << iter->first << ": ";
//    for (int var : *iter->second) {
//      cout << var << ", ";
//    }
//    cout << endl;
//  }
//  cout << endl;
//
  for(map<PooledLiterals*, DynamicLiterals*>::iterator iter = litsToAdd->begin(); iter != litsToAdd->end(); ++iter) {
    iter->first->println(stderr);
    iter->second->println(stderr);
    PooledLiterals* clause = iter->first;
    DynamicLiterals* literals = iter->second;
    for (Literal lit : *literals) {
      clause->add(~lit);
    }
  }

  if (litsToAdd->size() > 0)
    clauses->printDimacs();
}


