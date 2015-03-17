/*
 * MaxLengthFilter.cc
 *
 *  Created on: 05.06.2014
 *      Author: markus
 */

#include "ClauseFilter.h"
#include "ClauseFilters.h"
#include "../types/Literals.h"

class CardinalityFilter: public ClauseFilter {

private:
  Dark::Literals* model;
  int length;

public:
  CardinalityFilter(Dark::Literals* model, int length);
  virtual ~CardinalityFilter();
  virtual bool meetCriteria(Dark::Literals* clause);
};

CardinalityFilter::CardinalityFilter(Dark::Literals* model, int length) {
  this->model = model;
  this->length = length;
}

CardinalityFilter::~CardinalityFilter() { }

bool CardinalityFilter::meetCriteria(Dark::Literals* clause) {
  return model->cardinality(clause) < length;
}

/**
 * Implementation for IClauseFilters method
 */
std::unique_ptr<ClauseFilter> createCardinalityFilter(Dark::Literals* model, int length) {
  return std::unique_ptr<ClauseFilter>(new CardinalityFilter(model, length));
}
