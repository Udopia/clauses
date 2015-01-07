/*
 * MarkFilter.cc
 *
 *  Created on: Jun 2, 2014
 *      Author: markus
 */

#include "ClauseFilter.h"
#include "ClauseFilters.h"
#include "../types/Clause.h"

class MarkFilter: public ClauseFilter {

public:
  MarkFilter();
  virtual ~MarkFilter();
  virtual bool meetCriteria(Analyzer::Clause* clause);
  virtual void print();
};

MarkFilter::MarkFilter() { }

MarkFilter::~MarkFilter() { }

bool MarkFilter::meetCriteria(Analyzer::Clause* clause) {
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
