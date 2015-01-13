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

using namespace std;

namespace Dark {

class Cube;
class Clause;
class ClauseList;
class MappedClauseList;

class GateAnalyzer {
public:
  GateAnalyzer(ClauseList* clauseList, bool use_refinement = false);
  virtual ~GateAnalyzer();

  void analyzeEncoding();

  /**
   * Access Gate-Structure
   */
  ClauseList* getRoots();
  set<Literal>* getChildren(Literal parent);
  bool hasChildren(Literal parent);
  set<Literal>* getParents(Literal child);
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

private:
  MappedClauseList* clauses;

  // parent-child relationship (children are inputs, parents are outputs)
  map<Literal, set<Literal>*>* children;
  map<Literal, set<Literal>*>* parents;
  // for each output-Literal the defining clauses
  map<Literal, ClauseList*>* forwardClauses;
  map<Literal, ClauseList*>* backwardClauses;
  Projection* projection;

  bool use_refinement;

  void analyzeEncoding(Literal root);

  void setParentChild(Literal parent, Literal child);
  void unsetParentChild(Literal parent);

  bool classifyEncoding(Literal Literaleral);
  bool isBlocked(Literal Literaleral, Dark::Clause* incoming_clause, Dark::ClauseList* outgoing);
  bool isGate(Literal Literaleral, Dark::ClauseList* forward, Dark::ClauseList* backward);

  void setAsGate(Literal output, Dark::ClauseList* definition);
  bool isMonotonous(Literal Literaleral);

  void freeAllContent();
};

}

#endif /* GATEANALYZER_H_ */
