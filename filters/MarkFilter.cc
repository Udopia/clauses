/*
 * MarkFilter.cc
 *
 *  Created on: Jun 2, 2014
 *      Author: markus
 */

#include "../types/PooledLiterals.h"
#include "ClauseFilter.h"
#include "ClauseFilters.h"

class MarkFilter: public ClauseFilter {

public:
  MarkFilter();
  virtual ~MarkFilter();
  virtual bool meetCriteria(Dark::PooledLiterals* clause);
  virtual void print();
};

MarkFilter::MarkFilter() { }

MarkFilter::~MarkFilter() { }

bool MarkFilter::meetCriteria(Dark::PooledLiterals* clause) {
  return clause->isMarked();
}

void MarkFilter::print() {
  printf("MARKED(clause)");
}

/**
 * Implementation for IClauseFilters method
 */
std::unique_ptr<ClauseFilter> createMarkFilter() {
  return std::unique_ptr<ClauseFilter>(new MarkFilter());
}
