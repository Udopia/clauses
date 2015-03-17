/*
 * UnitFilter.cc
 *
 *  Created on: Jun 2, 2014
 *      Author: markus
 */

#include "ClauseFilter.h"
#include "ClauseFilters.h"
#include "../types/Literals.h"

class UnitFilter: public ClauseFilter {

public:
  UnitFilter();
  virtual ~UnitFilter();
  virtual bool meetCriteria(Dark::Literals* clause);
};

UnitFilter::UnitFilter() { }

UnitFilter::~UnitFilter() { }

bool UnitFilter::meetCriteria(Dark::Literals* clause) {
  return clause->size() == 1;
}

/**
 * Implementation for IClauseFilters method
 */
std::unique_ptr<ClauseFilter> createUnitFilter() {
  return std::unique_ptr<ClauseFilter>(new UnitFilter());
}
