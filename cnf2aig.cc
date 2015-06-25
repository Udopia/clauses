//============================================================================
// Name        : cnf2aig.cpp
// Author      : Markus Iser
// Version     :
// Copyright   : Public Domain
//============================================================================

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <assert.h>

#include <stdarg.h>

#define VERBOSITY 0
#include "misc/debug.h"
#include "misc/Dimacs.h"

#include "tools/GateAnalyzer.h"

#include "types/Literal.h"
#include "types/Literals.h"
#include "types/ClauseList.h"

#include "filters/ClauseFilters.h"

using namespace Dark;

const Literal lit_True = { -3 };

class BinaryAndGate {
public:
  Literal output;
  Literal left;
  Literal right;

  void println() {
    if (right == lit_True) {
      fprintf(stderr, "%s%i = AND(%s%i, T)\n", sign(output)?"-":"", var(output)+1, sign(left)?"-":"", var(left)+1);
    } else {
      fprintf(stderr, "%s%i = AND(%s%i, %s%i)\n", sign(output)?"-":"", var(output)+1, sign(left)?"-":"", var(left)+1, sign(right)?"-":"", var(right)+1);
    }
  }
};

class AndGate {
public:
  Literal output;
  Literals* conj;

  void println() {
    fprintf(stderr, "%s%i = AND%s\n", sign(output)?"-":"", var(output)+1, conj->toString()->c_str());
  }
};

Dimacs* problem;
GateAnalyzer* gates;
Literal root;
set<Var>* inputs = new set<Var>();

vector<BinaryAndGate*>* ands = new vector<BinaryAndGate*>();
vector<AndGate*>* pands = new vector<AndGate*>();

vector<Literal>* literals;
vector<bool>* visitedNodes;

int maxVariable = 0;
vector<bool>* negativeOutput;

int gaplessMax = 0;
map<Var, Var>* gapClosing = new map<Var, Var>();

int newVar() {
  return ++maxVariable;
}

bool isVisited(Literal node) {
  return (*visitedNodes)[toInt(node)];
}

void setVisited(Literal node) {
  (*visitedNodes)[toInt(node)] = true;
}

void createAnd(Literal output, Literals* cube) {
  AndGate* andGate = new AndGate();
  andGate->output = output;
  andGate->conj = cube;
  pands->push_back(andGate);
  D2(fprintf(stderr, "And created: ");
  andGate->println();)
}

void createBinaryAnd(Literal output, Literal left, Literal right) {
  BinaryAndGate* andGate = new BinaryAndGate();
  andGate->output = output;
  andGate->left = left;
  andGate->right = right;
  ands->push_back(andGate);
  D2(fprintf(stderr, "And created: ");
  andGate->println();)
}

void registerLiteral(Literal lit);
void createAndFromClause(Dark::Literals* disj, Literal output);
void createAndFromClauses(ClauseList* disjunctions, Literal output);

/**
 * Maintain Invariant: Mapping exists only when gate is defined
 */
void traverseDAG() {
  while (literals->size() > 0) {
    Literal lit = literals->back();
    literals->pop_back();

    if (isVisited(lit)) continue;
    setVisited(lit);

    D1(fprintf(stderr, "Literal %s%i\n", sign(lit)?"-":"", var(lit)+1));

    if (gates->hasInputs(lit)) {
      ClauseList* clauses = gates->getGateClauses(lit);

      D1(fprintf(stderr, "create aig from gate: ");
      clauses->print(stderr);
      fprintf(stderr, "\n");)

      // strip output-literal from clauses
      ClauseList* list = new ClauseList();
      for (unsigned int i = 0; i < clauses->size(); i++) {
        list->add(clauses->get(i)->allBut(~lit));
      }

      createAndFromClauses(list, lit);

      list->freeClauses();
      delete list;
    }
  }
}

void createAndFromClauses(ClauseList* disjunctions, Literal output) {
  D1(fprintf(stderr, "output is %s%i. create aig from clauses: ", sign(output)?"-":"", var(output)+1);
  if (disjunctions->size() < 100) disjunctions->print();
  else fprintf(stderr, "too many clauses, output truncated");
  fprintf(stderr, "\n");)

  Literals* conj = new Literals();
  for (ClauseList::iterator clit = disjunctions->begin(); clit != disjunctions->end(); clit++) {
    Dark::Literals* clause = *clit;
    if (clause->size() == 1) {
      registerLiteral(clause->getFirst());
      conj->add(clause->getFirst());
    } else {
      Literal out = mkLit(newVar());
      createAndFromClause(clause, out);
      conj->add(out);
    }
  }

  createAnd(output, conj);
}

void createAndFromClause(Dark::Literals* disj, Literal output) {
  D2(fprintf(stderr, "output is %s%i. create aig from clause: %s\n", sign(output)?"-":"", var(output)+1, disj->toString()->c_str()));

  Literals* conj = new Literals();
  for (Dark::Literals::iterator clit = disj->begin(); clit != disj->end(); clit++) {
    Literal lit = *clit;
    registerLiteral(lit);
    conj->add(~lit);
  }

  createAnd(~output, conj);
}

void registerLiteral(Literal lit) {
  if (gates->hasInputs(lit)) {
    D2(fprintf(stderr, "Enqueue Literal %s%i\n", sign(lit)?"-":"", var(lit)+1));
    literals->push_back(lit);
  } else if (gates->hasInputs(~lit)) {
    D2(fprintf(stderr, "Enqueue Literal %s%i\n", sign(lit)?"-":"", var(lit)+1));
    literals->push_back(~lit);
  } else {
    D2(fprintf(stderr, "New Input Literal %s%i\n", sign(lit)?"-":"", var(lit)+1));
    inputs->insert(var(lit));
  }
}

Literal convertNaryRecursive(Literals* conj) {
  if (conj->size() == 2) {
    Literal output = mkLit(newVar());
    createBinaryAnd(output, conj->getFirst(), conj->getLast());
    return output;
  }
  else if (conj->size() == 3) {
    Literal output = mkLit(newVar());
    Literal right = conj->removeLast();
    Literal left = convertNaryRecursive(conj);
    createBinaryAnd(output, left, right);
    return output;
  } else {
    Literal output = mkLit(newVar());
    Literals* lefts = conj->slice(0, conj->size() / 2);
    Literal left = convertNaryRecursive(lefts);
    delete lefts;
    Literals* rights = conj->slice(conj->size() / 2, conj->size());
    Literal right = convertNaryRecursive(rights);
    delete rights;
    createBinaryAnd(output, left, right);
    return output;
  }
}

void convertNaryAndsToBinaryAnds() {
  for (AndGate* a : *pands) {
    if (a->conj->size() == 0) {
      fprintf(stderr, "Warning: Conjunction Size is Zero at %i\n ", toInt(a->output));
    }
    else if (a->conj->size() == 1) {
      createBinaryAnd(a->output, a->conj->getFirst(), lit_True);
    }
    else if (a->conj->size() == 2) {
      createBinaryAnd(a->output, a->conj->getFirst(), a->conj->getLast());
    }
    else if (a->conj->size() == 3) {
      Literal right = a->conj->removeLast();
      Literal left = convertNaryRecursive(a->conj);
      createBinaryAnd(a->output, left, right);
    }
    else {
      Literals* lefts = a->conj->slice(0, a->conj->size() / 2);
      Literal left = convertNaryRecursive(lefts);
      delete lefts;
      Literals* rights = a->conj->slice(a->conj->size() / 2, a->conj->size());
      Literal right = convertNaryRecursive(rights);
      delete rights;
      createBinaryAnd(a->output, left, right);
    }
  }
}

/**
 * remember output negation
 */
Literal negOutAdaption(Literal lit) {
  return (*negativeOutput)[var(lit)] ? ~lit : lit;
}

/**
 * close gaps in variable numbers
 */
Literal closeGaps(Literal lit) {
  if (gapClosing->count(var(lit)) > 0) {
    return mkLit((*gapClosing)[var(lit)], sign(lit));
  }
  else {
    (*gapClosing)[var(lit)] = gaplessMax++;
    return mkLit((*gapClosing)[var(lit)], sign(lit));
  }
}

/**
 * print literal while selecting the proper sign and variable number
 */
void printLit(FILE* out, Literal lit) {
  if (lit == lit_True) {
    fprintf(out, "1");
  } else {
    int num = 2+toInt(closeGaps(negOutAdaption(lit)));
    fprintf(out, "%i", num);
  }
}

bool hasEmptyClause(ClauseList* clauses) {
  for (ClauseList::iterator it = clauses->begin(); it != clauses->end(); it++) {
    if ((*it)->size() == 0) return true;
  }
  return false;
}


int main(int argc, char** argv) {
  if (argc == 1) {
    fprintf(stderr, "Usage: cnf2aig [parameters] file\n");
    exit(1);
  }

  FILE* out = stdout;
  char* filename = argv[1];
  RootSelectionMethod method = MIN_OCCURENCE;
  int tries = 1;
  int eqd = 1;
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
      eqd = atoi(argv[++i]);
    }
    else if (strcmp(argv[i], "-t") == 0 && i < argc - 1) {
      tries = atoi(argv[++i]);
    }
    else if (strcmp(argv[i], "-o") == 0 && i < argc - 1) {
      out = fopen(argv[++i], "wb");
    }
    else if (strcmp(argv[i], "-h") == 0) {
      help = true;
    }
    else {
      filename = argv[i];
    }
  }

  if (help) {
    fprintf(stderr, "Usage: cnf2aig [parameters] file\n");
    fprintf(stderr, "Parameters:\n");
    fprintf(stderr, "-m [number] (0 - first clause, 1 - max id, 2 - min occurence)\n");
    fprintf(stderr, "-t [number of tries]\n");
    fprintf(stderr, "-o [output filename]\n");
    fprintf(stderr, "-h show this help\n");
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

  if (problem->getRealNClauses() == 0) {
    fprintf(out, "aag 0 0 0 1 0\n1\n"); // trivial SAT
    exit(0);
  }
  else if (problem->getRealNVars() == 0 || hasEmptyClause(problem->getClauses())) {
    fprintf(out, "aag 0 0 0 1 0\n0\n"); // trivial UNSAT
    exit(0);
  }

  visitedNodes = new vector<bool>((problem->getRealNVars()+2)*2, false);

  ClauseList* clauses = problem->getClauses();
  gates = new GateAnalyzer(clauses, eqd);
  gates->analyzeEncoding(method, tries);

  maxVariable = gates->getClauses()->maxVar()+1;

  literals = new vector<Literal>();
  root = gates->getRoot();
  literals->push_back(root);

  assert(var(root) < maxVariable);

  traverseDAG();

  D1(for (AndGate* a : *pands) { a->println(); })

  convertNaryAndsToBinaryAnds();

  /*******
   * As output may not be negative in AIG format we need to flip these globally:
   ***/
  negativeOutput = new vector<bool>(maxVariable+2, false);
  for (BinaryAndGate* a : *ands) {
    if (sign(a->output)) (*negativeOutput)[var(a->output)] = true;
  }

  // **************
  // ** Print AIG ****
  // ***************

  fprintf(out, "aag %i %i %i %i %i\n", maxVariable+2, (int)inputs->size(), 0, 1, (int)ands->size());
  for (Var var : *inputs) {
    printLit(out, mkLit(var));
    fprintf(out, "\n");
  }
  printLit(out, root);
  fprintf(out, "\n");

  for (BinaryAndGate* a : *ands) {
    printLit(out, a->output);
    fprintf(out, " ");
    printLit(out, a->left);
    fprintf(out, " ");
    printLit(out, a->right);
    fprintf(out, "\n");
  }
}


