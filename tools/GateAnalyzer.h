/*
 * SolverMetaInfo.h
 *
 *  Created on: 04.05.2013
 *      Author: markus
 */

#ifndef GATEANALYZER_H_
#define GATEANALYZER_H_

#include <vector>
#include <map>
#include <bitset>
#include <set>
#include <algorithm>

#include "Projection.h"
#include "../types/Literal.h"
#include "../types/ClauseList.h"

#include "../minisat/MinisatSolver.h"

using namespace std;

//class MinisatSolver;

namespace Dark {

class PooledLiterals;
class ClauseIndex;
class Gate;

enum RootSelectionMethod {
  MIN_OCCURENCE, PURITY
};

enum EquivalenceDetectionMethod {
  SKIP, PATTERNS, SEMANTIC
};

class GateAnalyzer {
public:
  GateAnalyzer(ClauseList* clauseList, RootSelectionMethod sel = MIN_OCCURENCE, EquivalenceDetectionMethod eq = PATTERNS);
  virtual ~GateAnalyzer();

  void analyzeEncoding(int tries);

  Literal getRoot();
  Gate* getGate(Literal output);
  Gate* getOrCreateGate(Literal output);

  vector<Literal>* getInputs(Literal parent);
  bool hasInputs(Literal parent);

  vector<Literal>* getParents(Literal child);

  void setAsInput(Literal literal);
  bool isMonotonousInput(Var var);

private:
  RootSelectionMethod selectionMethod;
  EquivalenceDetectionMethod equivalenceDetectionMethod;

  ClauseList* clauses;
  ClauseIndex* index;

  MinisatSolver* minisat;

  map<Literal, vector<Literal>*>* parents;
  map<Literal, bool>* inputs;

  vector<Gate*>* gates;

  Literal root;

  MinisatSolver* getMinisatSolver();

  void analyzeEncodingForRoot(Literal root);

  void setParent(Literal parent, Literal child);

  Gate* defGate(Literal output, ClauseList* fwd, ClauseList* bwd);

  bool isFullEncoding(Literal output, ClauseList* fwd, ClauseList* bwd);
  bool semanticCheck(Literal output, ClauseList* fwd);
  bool increment(vector<int>& positions, vector<int> maxima);

  ClauseList* getNextClauses(ClauseList* list);

  Literal createRoot(ClauseList* clauses);
};

}

#endif /* GATEANALYZER_H_ */
