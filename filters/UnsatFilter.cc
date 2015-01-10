/*
 * SatFilter.cc
 *
 *  Created on: 05.06.2014
 *      Author: markus
 */

#include "ClauseFilter.h"
#include "ClauseFilters.h"
#include "../types/Clause.h"
#include "../types/Cube.h"

class UnsatFilter: public ClauseFilter {

private:
  Dark::Cube* model;

public:
  UnsatFilter(Dark::Cube* model);
  virtual ~UnsatFilter();
  virtual bool meetCriteria(Dark::Clause* clause);
};

UnsatFilter::UnsatFilter(Dark::Cube* model) {
  this->model = model;
}

UnsatFilter::~UnsatFilter() { }

bool UnsatFilter::meetCriteria(Dark::Clause* clause) {
  return model->falsifies(clause);
}

/**
 * Implementation for IClauseFilters method
 */
std::unique_ptr<ClauseFilter> createUnsatFilter(Dark::Cube* model) {
  return std::unique_ptr<ClauseFilter>(new UnsatFilter(model));
}


