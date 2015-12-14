/*
 * MaxLengthFilter.cc
 *
 *  Created on: 05.06.2014
 *      Author: markus
 */

#include "../types/PooledLiterals.h"
#include "ClauseFilter.h"
#include "ClauseFilters.h"

class CardinalityFilter: public ClauseFilter {

private:
  Dark::PooledLiterals* model;
  int length;

public:
  CardinalityFilter(Dark::PooledLiterals* model, int length);
  virtual ~CardinalityFilter();
  virtual bool meetCriteria(Dark::PooledLiterals* clause);
};

CardinalityFilter::CardinalityFilter(Dark::PooledLiterals* model, int length) {
  this->model = model;
  this->length = length;
}

CardinalityFilter::~CardinalityFilter() { }

bool CardinalityFilter::meetCriteria(Dark::PooledLiterals* clause) {
  return model->cardinality(clause) < length;
}

/**
 * Implementation for IClauseFilters method
 */
std::unique_ptr<ClauseFilter> createCardinalityFilter(Dark::PooledLiterals* model, int length) {
  return std::unique_ptr<ClauseFilter>(new CardinalityFilter(model, length));
}
