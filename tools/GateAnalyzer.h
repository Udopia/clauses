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
  GateAnalyzer(ClauseList* clauseList);
  virtual ~GateAnalyzer();

  void analyzeEncoding(RootSelectionMethod selectionMethod, EquivalenceDetectionMethod eqivalence, int tries);

  /**
   * Access Gate-Structure
   */
  Literal getRoot();
  Gate* getGate(Literal output);
  Gate* getOrCreateGate(Literal output);
  vector<Literal>* getInputs(Literal parent);
  int countInputs(Literal parent);
  bool hasInputs(Literal parent);
  vector<Literal>* getParents(Literal child);
  int countParents(Literal child);
  bool hasParents(Literal child);
  ClauseList* getGateClauses(Literal literal);

  void setAsInput(Literal literal);
  bool isUsedAsInput(Literal literal);
  bool isMonotonousInput(Var var);

  void setProjection(Projection* projection);
  bool projectionContains(Var var);

  ClauseList* getClauses() {
    return (ClauseList*)clauses;
  }


private:
  ClauseList* clauses;
  ClauseIndex* index;

  MinisatSolver* minisat;

  map<Literal, vector<Literal>*>* parents;
  map<Literal, bool>* inputs;

  vector<Gate*>* gates;

  Literal root;

  Projection* projection;

  MinisatSolver* getMinisatSolver();

  void analyzeEncodingForRoot(Literal root, EquivalenceDetectionMethod eqivalence);

  void setParent(Literal parent, Literal child);
  void unsetParent(Literal parent);

  Gate* defGate(Literal output, ClauseList* fwd, ClauseList* bwd);
  void undefGate(Gate* gate);

  bool isFullEncoding(Literal output, ClauseList* fwd, ClauseList* bwd, EquivalenceDetectionMethod equivalence);
  bool semanticCheck(Literal output, ClauseList* fwd);
  bool increment(vector<int>& positions, vector<int> maxima);

  ClauseList* getNextClauses(ClauseList* list, RootSelectionMethod method);
};

}

#endif /* GATEANALYZER_H_ */
