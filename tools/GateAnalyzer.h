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

class GateAnalyzer {
public:
  GateAnalyzer(ClauseList* clauseList, bool use_refinement = false);
  virtual ~GateAnalyzer();

  void analyzeEncoding();
  void analyzeEncoding2();

  /**
   * Access Gate-Structure
   */
  ClauseList* getRoots();
  set<Literal>* getChildren(Literal parent);
  int countChildren(Literal parent);
  bool hasChildren(Literal parent);
  set<Literal>* getParents(Literal child);
  int countParents(Literal child);
  bool hasParents(Literal child);
  ClauseList* getGateClauses(Literal Literaleral);
  ClauseList* getNotMarkedClauses();

  void setProjection(Projection* projection);
  void removeProjection();
  bool projectionContains(Var var);


  ClauseList* getSideProblem();
  ClauseList* getGateProblem();
  ClauseList* getPrunedGateProblem(Cube* model);
  ClauseList* getAllClauses();
  ClauseList* getPGClauses();

private:
  MappedClauseList* clauses;

  // parent-child relationship (children are inputs, parents are outputs)
  map<Literal, set<Literal>*>* children;
  map<Literal, set<Literal>*>* parents;

  // store gates by output-variable
  vector<Gate*>* gates;

  Projection* projection;

  bool use_refinement = false;
  int max_var = -1;

  int maxVar();
  int newVar();

  void analyzeEncoding(Literal root);

  void setParentChild(Literal parent, Literal child);
  void unsetParentChild(Literal parent);

  bool classifyEncoding(Literal literal);
  bool isGate(Literal literal, Dark::ClauseList* forward, Dark::ClauseList* backward);
  bool isMonotonous(Literal literal);

  void freeAllContent();
};

}

#endif /* GATEANALYZER_H_ */
