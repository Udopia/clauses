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

class SatFilter: public ClauseFilter {

private:
  Analyzer::Cube* model;

public:
  SatFilter(Analyzer::Cube* model);
  virtual ~SatFilter();
  virtual bool meetCriteria(Analyzer::Clause* clause);
};

SatFilter::SatFilter(Analyzer::Cube* model) {
  this->model = model;
}

SatFilter::~SatFilter() { }

bool SatFilter::meetCriteria(Analyzer::Clause* clause) {
  return model->satisfies(clause);
}

/**
 * Implementation for IClauseFilters method
 */
std::unique_ptr<ClauseFilter> createSatFilter(Analyzer::Cube* model) {
  return std::unique_ptr<ClauseFilter>(new SatFilter(model));
}


