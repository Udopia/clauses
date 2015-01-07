/*
 * UnitFilter.cc
 *
 *  Created on: Jun 2, 2014
 *      Author: markus
 */

#include "ClauseFilter.h"
#include "ClauseFilters.h"
#include "../types/Clause.h"

class UnitFilter: public ClauseFilter {

public:
  UnitFilter();
  virtual ~UnitFilter();
  virtual bool meetCriteria(Analyzer::Clause* clause);
};

UnitFilter::UnitFilter() { }

UnitFilter::~UnitFilter() { }

bool UnitFilter::meetCriteria(Analyzer::Clause* clause) {
  return clause->size() == 1;
}

/**
 * Implementation for IClauseFilters method
 */
std::unique_ptr<ClauseFilter> createUnitFilter() {
  return std::unique_ptr<ClauseFilter>(new UnitFilter());
}
