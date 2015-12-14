/*
 * SatFilter.cc
 *
 *  Created on: 05.06.2014
 *      Author: markus
 */

#include "../types/PooledLiterals.h"
#include "ClauseFilter.h"
#include "ClauseFilters.h"

class SatFilter: public ClauseFilter {

private:
  Dark::PooledLiterals* model;

public:
  SatFilter(Dark::PooledLiterals* model);
  virtual ~SatFilter();
  virtual bool meetCriteria(Dark::PooledLiterals* clause);
};

SatFilter::SatFilter(Dark::PooledLiterals* model) {
  this->model = model;
}

SatFilter::~SatFilter() { }

bool SatFilter::meetCriteria(Dark::PooledLiterals* clause) {
  return model->satisfies(clause);
}

/**
 * Implementation for IClauseFilters method
 */
std::unique_ptr<ClauseFilter> createSatFilter(Dark::PooledLiterals* model) {
  return std::unique_ptr<ClauseFilter>(new SatFilter(model));
}


