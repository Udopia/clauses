/*
 * MaxLengthFilter.cc
 *
 *  Created on: 05.06.2014
 *      Author: markus
 */

#include "ClauseFilter.h"
#include "ClauseFilters.h"
#include "../types/Clause.h"
#include "../types/Cube.h"

class CardinalityFilter: public ClauseFilter {

private:
  Dark::Cube* model;
  int length;

public:
  CardinalityFilter(Dark::Cube* model, int length);
  virtual ~CardinalityFilter();
  virtual bool meetCriteria(Dark::Clause* clause);
};

CardinalityFilter::CardinalityFilter(Dark::Cube* model, int length) {
  this->model = model;
  this->length = length;
}

CardinalityFilter::~CardinalityFilter() { }

bool CardinalityFilter::meetCriteria(Dark::Clause* clause) {
  return model->cardinality(clause) < length;
}

/**
 * Implementation for IClauseFilters method
 */
std::unique_ptr<ClauseFilter> createCardinalityFilter(Dark::Cube* model, int length) {
  return std::unique_ptr<ClauseFilter>(new CardinalityFilter(model, length));
}
