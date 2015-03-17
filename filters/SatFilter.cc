/*
 * SatFilter.cc
 *
 *  Created on: 05.06.2014
 *      Author: markus
 */

#include "ClauseFilter.h"
#include "ClauseFilters.h"
#include "../types/Literals.h"

class SatFilter: public ClauseFilter {

private:
  Dark::Literals* model;

public:
  SatFilter(Dark::Literals* model);
  virtual ~SatFilter();
  virtual bool meetCriteria(Dark::Literals* clause);
};

SatFilter::SatFilter(Dark::Literals* model) {
  this->model = model;
}

SatFilter::~SatFilter() { }

bool SatFilter::meetCriteria(Dark::Literals* clause) {
  return model->satisfies(clause);
}

/**
 * Implementation for IClauseFilters method
 */
std::unique_ptr<ClauseFilter> createSatFilter(Dark::Literals* model) {
  return std::unique_ptr<ClauseFilter>(new SatFilter(model));
}


