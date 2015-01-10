/*
 * SatFilter.cc
 *
 *  Created on: 05.06.2014
 *      Author: markus
 */

#include "ClauseFilter.h"
#include "ClauseFilters.h"
#include "../Clause.h"
#include "../Cube.h"

class SatFilter: public ClauseFilter {

private:
  Dark::Cube* model;

public:
  SatFilter(Dark::Cube* model);
  virtual ~SatFilter();
  virtual bool meetCriteria(Dark::Clause* clause);
};

SatFilter::SatFilter(Dark::Cube* model) {
  this->model = model;
}

SatFilter::~SatFilter() { }

bool SatFilter::meetCriteria(Dark::Clause* clause) {
  return model->satisfies(clause);
}

/**
 * Implementation for IClauseFilters method
 */
std::unique_ptr<ClauseFilter> createSatFilter(Dark::Cube* model) {
  return std::unique_ptr<ClauseFilter>(new SatFilter(model));
}


