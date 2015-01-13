/*
 * HornFilter.cc
 *
 *  Created on: 05.06.2014
 *      Author: markus
 */

#include "ClauseFilter.h"
#include "ClauseFilters.h"
#include "../types/Clause.h"
#include "../types/Literal.h"

class HornFilter: public ClauseFilter {

public:
  HornFilter();
  virtual ~HornFilter();
  virtual bool meetCriteria(Dark::Clause* clause);
};

HornFilter::HornFilter() { }

HornFilter::~HornFilter() { }

bool HornFilter::meetCriteria(Dark::Clause* clause) {
  int count = 0;
  for (Dark::Clause::iterator lit = clause->begin(); lit != clause->end(); lit++) {
    if (!sign(*lit)) {
      count++;
      if (count > 1) {
        return false;
      }
    }
  }
  return count <= 1;
}

/**
 * Implementation for IClauseFilters method
 */
std::unique_ptr<ClauseFilter> createHornFilter() {
  return std::unique_ptr<ClauseFilter>(new HornFilter());
}

