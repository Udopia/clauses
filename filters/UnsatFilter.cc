/*
 * SatFilter.cc
 *
 *  Created on: 05.06.2014
 *      Author: markus
 */

#include "../types/DynamicLiterals.h"
#include "ClauseFilter.h"
#include "ClauseFilters.h"

class UnsatFilter: public ClauseFilter {

private:
  Dark::DynamicLiterals* model;

public:
  UnsatFilter(Dark::DynamicLiterals* model);
  virtual ~UnsatFilter();
  virtual bool meetCriteria(Dark::DynamicLiterals* clause);
};

UnsatFilter::UnsatFilter(Dark::DynamicLiterals* model) {
  this->model = model;
}

UnsatFilter::~UnsatFilter() { }

bool UnsatFilter::meetCriteria(Dark::DynamicLiterals* clause) {
  return model->falsifies(clause);
}

/**
 * Implementation for IClauseFilters method
 */
std::unique_ptr<ClauseFilter> createUnsatFilter(Dark::DynamicLiterals* model) {
  return std::unique_ptr<ClauseFilter>(new UnsatFilter(model));
}


