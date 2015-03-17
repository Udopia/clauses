/*
 * SatFilter.cc
 *
 *  Created on: 05.06.2014
 *      Author: markus
 */

#include "ClauseFilter.h"
#include "ClauseFilters.h"
#include "../types/Literals.h"

class UnsatFilter: public ClauseFilter {

private:
  Dark::Literals* model;

public:
  UnsatFilter(Dark::Literals* model);
  virtual ~UnsatFilter();
  virtual bool meetCriteria(Dark::Literals* clause);
};

UnsatFilter::UnsatFilter(Dark::Literals* model) {
  this->model = model;
}

UnsatFilter::~UnsatFilter() { }

bool UnsatFilter::meetCriteria(Dark::Literals* clause) {
  return model->falsifies(clause);
}

/**
 * Implementation for IClauseFilters method
 */
std::unique_ptr<ClauseFilter> createUnsatFilter(Dark::Literals* model) {
  return std::unique_ptr<ClauseFilter>(new UnsatFilter(model));
}


