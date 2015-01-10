/*
 * UnitFilter.h
 *
 *  Created on: Jun 2, 2014
 *      Author: markus
 */

#ifndef UNITFILTER_H_
#define UNITFILTER_H_

#include <memory>

#include "ClauseFilter.h"

namespace Dark {
  class Cube;
}

std::unique_ptr<ClauseFilter> createUnitFilter();
std::unique_ptr<ClauseFilter> createBinaryFilter();
std::unique_ptr<ClauseFilter> createTernaryFilter();
std::unique_ptr<ClauseFilter> createMaxLengthFilter(int length);

std::unique_ptr<ClauseFilter> createHornFilter();

std::unique_ptr<ClauseFilter> createSatFilter(Dark::Cube* model);
std::unique_ptr<ClauseFilter> createUnsatFilter(Dark::Cube* model);
std::unique_ptr<ClauseFilter> createCardinalityFilter(Dark::Cube* model, int length);

std::unique_ptr<ClauseFilter> createConstantTrueFilter();

std::unique_ptr<ClauseFilter> createNotFilter(std::unique_ptr<ClauseFilter> argument);
std::unique_ptr<ClauseFilter> createAndFilter(std::unique_ptr<ClauseFilter> left, std::unique_ptr<ClauseFilter> right);
std::unique_ptr<ClauseFilter> createOrFilter(std::unique_ptr<ClauseFilter> left, std::unique_ptr<ClauseFilter> right);

#endif /* UNITFILTER_H_ */