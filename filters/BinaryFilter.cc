/*
 * BinaryFilter.cc
 *
 *  Created on: 05.06.2014
 *      Author: markus
 */

#include "ClauseFilter.h"
#include "ClauseFilters.h"
#include "../types/Clause.h"

class BinaryFilter: public ClauseFilter {

public:
  BinaryFilter();
  virtual ~BinaryFilter();
  virtual bool meetCriteria(Dark::Clause* clause);
};

BinaryFilter::BinaryFilter() { }

BinaryFilter::~BinaryFilter() { }

bool BinaryFilter::meetCriteria(Dark::Clause* clause) {
  return clause->size() == 2;
}

/**
 * Implementation for IClauseFilters method
 */
std::unique_ptr<ClauseFilter> createBinaryFilter() {
  return std::unique_ptr<ClauseFilter>(new BinaryFilter());
}



