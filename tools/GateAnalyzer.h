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

  vector<Literal>* getInputs(Literal parent);
  bool hasInputs(Literal parent);

  void setAsInput(Literal literal);
  bool isMonotonousInput(Var var);

private:
  RootSelectionMethod selectionMethod;
  EquivalenceDetectionMethod equivalenceDetectionMethod;

  ClauseList* clauses;

  Literal root;
  vector<Gate*>* gates;
  vector<bool>* inputs;

  MinisatSolver* minisat;

  MinisatSolver* getMinisatSolver();

  void analyzeEncodingBFS(Literal root, ClauseIndex* index);

  Gate* defGate(Literal output, ClauseList* fwd, ClauseList* bwd, bool monotonous);

  bool isFullEncoding(Literal output, ClauseList* fwd, ClauseList* bwd);
  bool semanticCheck(Literal output, ClauseList* fwd);
  bool increment(vector<int>& positions, vector<int> maxima);

  ClauseList* getNextClauses(ClauseIndex* index);
  void createRootGate(ClauseList* rootClauses);
};

}

#endif /* GATEANALYZER_H_ */
