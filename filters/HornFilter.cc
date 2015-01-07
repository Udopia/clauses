/*
 * HornFilter.cc
 *
 *  Created on: 05.06.2014
 *      Author: markus
 */

#include "ClauseFilter.h"
#include "ClauseFilters.h"
#include "../types/Clause.h"

class HornFilter: public ClauseFilter {

public:
  HornFilter();
  virtual ~HornFilter();
  virtual bool meetCriteria(Analyzer::Clause* clause);
};

HornFilter::HornFilter() { }

HornFilter::~HornFilter() { }

bool HornFilter::meetCriteria(Analyzer::Clause* clause) {
  int count = 0;
  for (Analyzer::Clause::iterator lit = clause->begin(); lit != clause->end(); lit++) {
    if (!sign(*lit)) count++;
  }
  return count <= 1;
}

/**
 * Implementation for IClauseFilters method
 */
std::unique_ptr<ClauseFilter> createHornFilter() {
  return std::unique_ptr<ClauseFilter>(new HornFilter());
}

