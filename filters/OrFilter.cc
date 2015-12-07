/*
 * OrFilter.cc
 *
 *  Created on: 05.06.2014
 *      Author: markus
 */

#include "../types/DynamicLiterals.h"
#include "ClauseFilter.h"
#include "ClauseFilters.h"

class OrFilter: public ClauseFilter {
private:
  std::unique_ptr<ClauseFilter> left;
  std::unique_ptr<ClauseFilter> right;

public:
  OrFilter(std::unique_ptr<ClauseFilter> left, std::unique_ptr<ClauseFilter> right);
  virtual ~OrFilter();
  virtual bool meetCriteria(Dark::DynamicLiterals* clause);
};

OrFilter::OrFilter(std::unique_ptr<ClauseFilter> left, std::unique_ptr<ClauseFilter> right) {
  this->left = std::move(left);
  this->right = std::move(right);
}

OrFilter::~OrFilter() { }

bool OrFilter::meetCriteria(Dark::DynamicLiterals* clause) {
  return left->meetCriteria(clause) || right->meetCriteria(clause);
}

/**
 * Implementation for IClauseFilters method
 */
std::unique_ptr<ClauseFilter> createOrFilter(std::unique_ptr<ClauseFilter> left, std::unique_ptr<ClauseFilter> right) {
  return std::unique_ptr<ClauseFilter>(new OrFilter(std::move(left), std::move(right)));
}

