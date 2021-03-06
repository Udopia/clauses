/*
 * ConstantTrueFilter.cc
 *
 *  Created on: Jun 2, 2014
 *      Author: markus
 */

#include "../types/PooledLiterals.h"
#include "ClauseFilter.h"
#include "ClauseFilters.h"

class ConstantTrueFilter: public ClauseFilter {

public:
  ConstantTrueFilter();
  virtual ~ConstantTrueFilter();
  virtual bool meetCriteria(Dark::PooledLiterals* clause);
};

ConstantTrueFilter::ConstantTrueFilter() { }

ConstantTrueFilter::~ConstantTrueFilter() { }

bool ConstantTrueFilter::meetCriteria(Dark::PooledLiterals* clause) {
  return clause == clause;
}

/**
 * Implementation for IClauseFilters method
 */
std::unique_ptr<ClauseFilter> createConstantTrueFilter() {
  return std::unique_ptr<ClauseFilter>(new ConstantTrueFilter());
}
