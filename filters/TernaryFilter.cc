/*
 * TernaryFilter.cc
 *
 *  Created on: 05.06.2014
 *      Author: markus
 */

#include "ClauseFilter.h"
#include "ClauseFilters.h"
#include "../types/Literals.h"

class TernaryFilter: public ClauseFilter {

public:
  TernaryFilter();
  virtual ~TernaryFilter();
  virtual bool meetCriteria(Dark::Literals* clause);
};

TernaryFilter::TernaryFilter() { }

TernaryFilter::~TernaryFilter() { }

bool TernaryFilter::meetCriteria(Dark::Literals* clause) {
  return clause->size() == 3;
}

/**
 * Implementation for IClauseFilters method
 */
std::unique_ptr<ClauseFilter> createTernaryFilter() {
  return std::unique_ptr<ClauseFilter>(new TernaryFilter());
}
