/*
 * SatFilter.cc
 *
 *  Created on: 05.06.2014
 *      Author: markus
 */

#include "../types/PooledLiterals.h"
#include "ClauseFilter.h"
#include "ClauseFilters.h"

class UnsatFilter: public ClauseFilter {

private:
  Dark::PooledLiterals* model;

public:
  UnsatFilter(Dark::PooledLiterals* model);
  virtual ~UnsatFilter();
  virtual bool meetCriteria(Dark::PooledLiterals* clause);
};

UnsatFilter::UnsatFilter(Dark::PooledLiterals* model) {
  this->model = model;
}

UnsatFilter::~UnsatFilter() { }

bool UnsatFilter::meetCriteria(Dark::PooledLiterals* clause) {
  return model->falsifies(clause);
}

/**
 * Implementation for IClauseFilters method
 */
std::unique_ptr<ClauseFilter> createUnsatFilter(Dark::PooledLiterals* model) {
  return std::unique_ptr<ClauseFilter>(new UnsatFilter(model));
}


