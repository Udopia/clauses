/*
 * MaxLengthFilter.cc
 *
 *  Created on: 05.06.2014
 *      Author: markus
 */

#include "../types/DynamicLiterals.h"
#include "ClauseFilter.h"
#include "ClauseFilters.h"

class CardinalityFilter: public ClauseFilter {

private:
  Dark::DynamicLiterals* model;
  int length;

public:
  CardinalityFilter(Dark::DynamicLiterals* model, int length);
  virtual ~CardinalityFilter();
  virtual bool meetCriteria(Dark::DynamicLiterals* clause);
};

CardinalityFilter::CardinalityFilter(Dark::DynamicLiterals* model, int length) {
  this->model = model;
  this->length = length;
}

CardinalityFilter::~CardinalityFilter() { }

bool CardinalityFilter::meetCriteria(Dark::DynamicLiterals* clause) {
  return model->cardinality(clause) < length;
}

/**
 * Implementation for IClauseFilters method
 */
std::unique_ptr<ClauseFilter> createCardinalityFilter(Dark::DynamicLiterals* model, int length) {
  return std::unique_ptr<ClauseFilter>(new CardinalityFilter(model, length));
}
