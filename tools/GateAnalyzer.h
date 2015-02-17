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

using namespace std;

namespace Dark {

class Cube;
class Clause;
class MappedClauseList;
class Gate;

enum RootSelectionMethod {
  FIRST_CLAUSE, MAX_ID, MIN_OCCURENCE, MONOTONOUS
};

class GateAnalyzer {
public:
  GateAnalyzer(ClauseList* clauseList, bool full_eq_detection = false, bool use_refinement = false);
  virtual ~GateAnalyzer();

  void analyzeEncoding(RootSelectionMethod method, int tries);

  /**
   * Access Gate-Structure
   */
  ClauseList* getRoots();
  Gate* getGate(Literal output);
  Gate* getOrCreateGate(Literal output);
  vector<Literal>* getInputs(Literal parent);
  int countInputs(Literal parent);
  bool hasInputs(Literal parent);
  vector<Literal>* getParents(Literal child);
  int countParents(Literal child);
  bool hasParents(Literal child);
  ClauseList* getGateClauses(Literal Literaleral);

  void setProjection(Projection* projection);
  bool projectionContains(Var var);

  ClauseList* getSideProblem();
  ClauseList* getGateProblem();
  ClauseList* getPrunedGateProblem(Cube* model);
  ClauseList* getAllClauses();
  ClauseList* getPGClauses(Literal root, bool monotonous);
  ClauseList* getPGClauses();

  vector<vector<Literal>*>* findConfluentOutputs();
  vector<int>* getRecursiveGateOrWidths();
  void augmentClauses(int minWidth = 0);

private:
  MappedClauseList* clauses;
  vector<bool>* visited;

  map<Literal, vector<Literal>*>* parents;

  // store gates by output-variable
  vector<Gate*>* gates;

  Projection* projection;

  bool use_refinement = false;
  bool full_eq_detection = false;
  int max_var = -1;

  int maxVar();
  int newVar();

  void analyzeEncoding(Literal root);

  void setParent(Literal parent, Literal child);
  void unsetParent(Literal parent);

  Gate* defGate(Literal output, ClauseList* fwd, ClauseList* bwd);
  void undefGate(Gate* gate);

  bool classifyEncoding(Literal literal);

  bool isMonotonousInput(Literal output);

  Clause* getNextClause(ClauseList* list, RootSelectionMethod method);

  void freeAllContent();
};

}

#endif /* GATEANALYZER_H_ */
