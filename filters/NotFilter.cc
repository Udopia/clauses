/*
 * NotFilter.cc
 *
 *  Created on: 05.06.2014
 *      Author: markus
 */

#include "../types/PooledLiterals.h"
#include "ClauseFilter.h"
#include "ClauseFilters.h"

class NotFilter: public ClauseFilter {
private:
  std::unique_ptr<ClauseFilter> argument;

public:
  NotFilter(std::unique_ptr<ClauseFilter> argument);
  virtual ~NotFilter();
  virtual bool meetCriteria(Dark::PooledLiterals* clause);
};

NotFilter::NotFilter(std::unique_ptr<ClauseFilter> argument) {
  this->argument = std::move(argument);
}

NotFilter::~NotFilter() { }

bool NotFilter::meetCriteria(Dark::PooledLiterals* clause) {
  return !argument->meetCriteria(clause);
}

/**
 * Implementation for IClauseFilters method
 */
std::unique_ptr<ClauseFilter> createNotFilter(std::unique_ptr<ClauseFilter> argument) {
  return std::unique_ptr<ClauseFilter>(new NotFilter(std::move(argument)));
}

