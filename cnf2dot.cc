/*
 * cnf2dot.cc
 *
 *  Created on: Jan 14, 2015
 *      Author: Markus Iser
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

#include <stdarg.h>

#define VERBOSITY 0
#include "misc/debug.h"
#include "misc/Dimacs.h"

#include "tools/GateAnalyzer.h"
#include "tools/Gate.h"

#include "types/Literal.h"
#include "types/Literals.h"
#include "types/ClauseList.h"
#include "types/MappedClauseList.h"

#include "filters/ClauseFilters.h"

using namespace Dark;

vector<bool>* visited;
int maxId = 0;

bool isVisited(Literal lit) {
  return (*visited)[toInt(lit)];
}

void setVisited(Literal lit) {
  (*visited)[toInt(lit)] = true;
}

void graphviz_create_child_nodes(GateAnalyzer* analyzer, Literal parent) {
  if (isVisited(parent)) return;
  setVisited(parent);

  Gate* gate = analyzer->getGate(parent);
  if (gate != NULL) {
    ClauseList* fwd = gate->getForwardClauses();
    int parentNodeId = 2*var(parent)+(sign(parent)?1:0);

    for (ClauseList::iterator clause = fwd->begin(); clause != fwd->end(); clause++) {
      if ((*clause)->size() < 3) {
        Literal literal = (*clause)->getFirst() == ~parent ? (*clause)->getLast() : (*clause)->getFirst();
        int childNodeId = 2*var(literal)+(sign(literal)?1:0);

        if (!isVisited(literal)) {
          if (analyzer->getGate(literal) != NULL) {
            fprintf(stdout, "  N%i [label=%s%i, shape=triangle]\n", childNodeId, sign(literal)?"-":"", var(literal)+1);
            graphviz_create_child_nodes(analyzer, literal);
          } else {
            fprintf(stdout, "  N%i [label=%s%i, shape=circle]\n", childNodeId, sign(literal)?"-":"", var(literal)+1);
          }
        }

        fprintf(stdout, "  N%i->N%i\n", parentNodeId, childNodeId);
      }
      else {
        int clauseNodeId = ++maxId;

        fprintf(stdout, "  N%i [label=\"\" shape=point]\n", clauseNodeId);
        fprintf(stdout, "  N%i->N%i\n", parentNodeId, clauseNodeId);

        for (Literals::iterator it2 = (*clause)->begin(); it2 != (*clause)->end(); it2++) {
          Literal literal = *it2;
          int childNodeId = 2*var(literal)+(sign(literal)?1:0);

          if (literal == ~parent) continue;

          if (!isVisited(literal)) {
            if (analyzer->getGate(literal) != NULL) {
              fprintf(stdout, "  N%i [label=%s%i, shape=triangle]\n", childNodeId, sign(literal)?"-":"", var(literal)+1);
              graphviz_create_child_nodes(analyzer, literal);
            } else {
              fprintf(stdout, "  N%i [label=%s%i, shape=circle]\n", childNodeId, sign(literal)?"-":"", var(literal)+1);
            }
          }

          fprintf(stdout, "  N%i->N%i\n", clauseNodeId, childNodeId);
        }
      }
    }
  }
}

void graphviz_create_dag(Dimacs* problem, GateAnalyzer* analyzer, Literal root) {
  fprintf(stdout, "  N%i [label=%s%i, shape=triangle]\n", 2*var(root)+(sign(root)?1:0), sign(root)?"-":"", var(root)+1);
  graphviz_create_child_nodes(analyzer, root);
}

void graphviz_create_info_nodes(GateAnalyzer* analyzer) {
  int vars = 2 + visited->size() / 2;
  for (int i = 0; i < vars; i++) {
    Literal literal = mkLit(i, false);
    Gate* gate = analyzer->getGate(literal);
    if (gate == NULL) {
      literal = ~literal;
      gate = analyzer->getGate(literal);
    }
    if (gate == NULL) continue;
    int nodeId = 2*var(literal)+(sign(literal)?1:0);
    int newNodeId = ++maxId;
    fprintf(stdout, "    N%i [shape=note label=\"", newNodeId);
    ClauseList* fwd = gate->getForwardClauses();
    for (ClauseList::iterator clause = fwd->begin(); clause != fwd->end(); clause++) {
      (*clause)->print(stdout);
      fprintf(stdout, "\\n");
    }
    fprintf(stdout, "---\\n");
    ClauseList* rwd = gate->getBackwardClauses();
    for (ClauseList::iterator clause = rwd->begin(); clause != rwd->end(); clause++) {
      (*clause)->print(stdout);
      fprintf(stdout, "\\n");
    }
    fprintf(stdout, "\"]\n");
    fprintf(stdout, "    N%i -> N%i [style=dotted]\n", nodeId, newNodeId);
  }
}

int main(int argc, char** argv) {
  if (argc == 1) {
    fprintf(stderr, "Usage: cnf2aig cnf-problem\n");
    exit(1);
  }

  bool verbose = argc > 2 && strcmp(argv[2], "-v") == 0;

  gzFile in = gzopen(argv[1], "rb");
  if (in == NULL) {
    fprintf(stderr, "Error: Could not open file: %s\n", argc == 1 ? "<stdin>" : argv[1]);
    exit(1);
  }
  Dimacs* problem = new Dimacs(in);
  gzclose(in);

  visited = new vector<bool>(2*problem->getRealNVars(), false);
  maxId = 2*problem->getRealNVars()+2;

  ClauseList* clauses = problem->getClauses();
  GateAnalyzer* gates = new GateAnalyzer(clauses, 1);
  gates->analyzeEncoding(MIN_OCCURENCE, 1);

  fprintf(stdout, "digraph {\n  label=\"Extracted Gate-Structure From CNF\"\n");
  Literal root = gates->getRoot();
  graphviz_create_dag(problem, gates, root);
  if (verbose) {
    fprintf(stdout, "  subgraph notes {\n");
    fprintf(stdout, "    node [style=filled, color=grey]\n");
    fprintf(stdout, "    edge [dir=none]\n");
    graphviz_create_info_nodes(gates);
    fprintf(stdout, "  }\n");
  }
  fprintf(stdout, "}\n");
}






