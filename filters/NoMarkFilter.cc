/*
 * MarkFilter.cc
 *
 *  Created on: Jun 2, 2014
 *      Author: markus
 */

#include "../types/DynamicLiterals.h"
#include "ClauseFilter.h"
#include "ClauseFilters.h"

class NoMarkFilter: public ClauseFilter {

public:
  NoMarkFilter();
  virtual ~NoMarkFilter();
  virtual bool meetCriteria(Dark::DynamicLiterals* clause);
  virtual void print();
};

NoMarkFilter::NoMarkFilter() { }

NoMarkFilter::~NoMarkFilter() { }

bool NoMarkFilter::meetCriteria(Dark::DynamicLiterals* clause) {
  return !clause->isMarked();
}

void NoMarkFilter::print() {
  printf("NOT_MARKED(clause)");
}

/**
 * Implementation for IClauseFilters method
 */
std::unique_ptr<ClauseFilter> createNoMarkFilter() {
  return std::unique_ptr<ClauseFilter>(new NoMarkFilter());
}
