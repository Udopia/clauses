/*
 * SatFilter.cc
 *
 *  Created on: 05.06.2014
 *      Author: markus
 */

#include "../types/DynamicLiterals.h"
#include "ClauseFilter.h"
#include "ClauseFilters.h"

class SatFilter: public ClauseFilter {

private:
  Dark::DynamicLiterals* model;

public:
  SatFilter(Dark::DynamicLiterals* model);
  virtual ~SatFilter();
  virtual bool meetCriteria(Dark::DynamicLiterals* clause);
};

SatFilter::SatFilter(Dark::DynamicLiterals* model) {
  this->model = model;
}

SatFilter::~SatFilter() { }

bool SatFilter::meetCriteria(Dark::DynamicLiterals* clause) {
  return model->satisfies(clause);
}

/**
 * Implementation for IClauseFilters method
 */
std::unique_ptr<ClauseFilter> createSatFilter(Dark::DynamicLiterals* model) {
  return std::unique_ptr<ClauseFilter>(new SatFilter(model));
}


