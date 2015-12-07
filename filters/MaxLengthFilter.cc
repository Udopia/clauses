/*
 * MaxLengthFilter.cc
 *
 *  Created on: 05.06.2014
 *      Author: markus
 */

#include "../types/DynamicLiterals.h"
#include "ClauseFilter.h"
#include "ClauseFilters.h"

class MaxLengthFilter: public ClauseFilter {
private:
  unsigned int length;

public:
  MaxLengthFilter(unsigned int length);
  virtual ~MaxLengthFilter();
  virtual bool meetCriteria(Dark::DynamicLiterals* clause);
};

MaxLengthFilter::MaxLengthFilter(unsigned int length) {
  this->length = length;
}

MaxLengthFilter::~MaxLengthFilter() { }

bool MaxLengthFilter::meetCriteria(Dark::DynamicLiterals* clause) {
  return clause->size() <= length;
}

/**
 * Implementation for IClauseFilters method
 */
std::unique_ptr<ClauseFilter> createMaxLengthFilter(int length) {
  return std::unique_ptr<ClauseFilter>(new MaxLengthFilter(length));
}
