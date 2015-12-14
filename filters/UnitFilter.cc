/*
 * UnitFilter.cc
 *
 *  Created on: Jun 2, 2014
 *      Author: markus
 */

#include "../types/PooledLiterals.h"
#include "ClauseFilter.h"
#include "ClauseFilters.h"

class UnitFilter: public ClauseFilter {

public:
  UnitFilter();
  virtual ~UnitFilter();
  virtual bool meetCriteria(Dark::PooledLiterals* clause);
};

UnitFilter::UnitFilter() { }

UnitFilter::~UnitFilter() { }

bool UnitFilter::meetCriteria(Dark::PooledLiterals* clause) {
  return clause->size() == 1;
}

/**
 * Implementation for IClauseFilters method
 */
std::unique_ptr<ClauseFilter> createUnitFilter() {
  return std::unique_ptr<ClauseFilter>(new UnitFilter());
}
