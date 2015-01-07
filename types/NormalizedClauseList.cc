/*
 * NormalizedClauseList.cpp
 *
 *  Created on: 10.01.2014
 *      Author: markus
 */

#include "NormalizedClauseList.h"

#include "ClauseList.h"
#include "Cube.h"

namespace Dark {

NormalizedClauseList::NormalizedClauseList(Cube* model) {
  this->model = model;
  this->denormalizedLiteral = new map<Minisat::Lit, Minisat::Lit>();

  this->nVars = 0;
  for (std::vector<Minisat::Lit>::iterator it = model->begin(); it != model->end(); ++it) {
    Minisat::Lit lit = *it;
    if (var(lit)+1 > this->nVars) this->nVars = var(lit)+1;
  }
}

NormalizedClauseList::~NormalizedClauseList() {
  freeClauses();
  delete denormalizedLiteral;
}

void NormalizedClauseList::add(Clause* clause) {
  Clause* normalizedClause = new Clause();
  for (std::vector<Minisat::Lit>::iterator it = clause->begin(); it != clause->end(); ++it) {
    Minisat::Lit lit = *it;
    if (model->contains(lit)) {
      normalizedClause->add(normalize(lit));
    }
  }

  ClauseList::add(normalizedClause);
}

void NormalizedClauseList::addAll(ClauseList* list) {
  for(int i = 0; i < list->size(); i++) {
    this->add(list->get(i));
  }
}

Minisat::Lit NormalizedClauseList::normalize(Minisat::Lit lit) {
  if (sign(lit)) {
    (*denormalizedLiteral)[lit] = ~lit;
    (*denormalizedLiteral)[~lit] = lit;
    return ~lit;
  } else {
    (*denormalizedLiteral)[lit] = lit;
    (*denormalizedLiteral)[~lit] = ~lit;
    return lit;
  }
}

Minisat::Lit NormalizedClauseList::denormalize(Minisat::Lit lit) {
  return (*denormalizedLiteral)[lit];
}

} /* namespace Analyzer */
