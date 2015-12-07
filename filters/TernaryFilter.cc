/*
 * TernaryFilter.cc
 *
 *  Created on: 05.06.2014
 *      Author: markus
 */

#include "../types/DynamicLiterals.h"
#include "ClauseFilter.h"
#include "ClauseFilters.h"

class TernaryFilter: public ClauseFilter {

public:
  TernaryFilter();
  virtual ~TernaryFilter();
  virtual bool meetCriteria(Dark::DynamicLiterals* clause);
};

TernaryFilter::TernaryFilter() { }

TernaryFilter::~TernaryFilter() { }

bool TernaryFilter::meetCriteria(Dark::DynamicLiterals* clause) {
  return clause->size() == 3;
}

/**
 * Implementation for IClauseFilters method
 */
std::unique_ptr<ClauseFilter> createTernaryFilter() {
  return std::unique_ptr<ClauseFilter>(new TernaryFilter());
}
