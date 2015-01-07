/*
 * AndFilter.cc
 *
 *  Created on: 05.06.2014
 *      Author: markus
 */

#include "ClauseFilter.h"
#include "ClauseFilters.h"
#include "../types/Clause.h"

class AndFilter: public ClauseFilter {
private:
  std::unique_ptr<ClauseFilter> left;
  std::unique_ptr<ClauseFilter> right;

public:
  AndFilter(std::unique_ptr<ClauseFilter> left, std::unique_ptr<ClauseFilter> right);
  virtual ~AndFilter();
  virtual bool meetCriteria(Analyzer::Clause* clause);
};

AndFilter::AndFilter(std::unique_ptr<ClauseFilter> left, std::unique_ptr<ClauseFilter> right) {
  this->left = std::move(left);
  this->right = std::move(right);
}

AndFilter::~AndFilter() { }

bool AndFilter::meetCriteria(Analyzer::Clause* clause) {
  return left->meetCriteria(clause) && right->meetCriteria(clause);
}

/**
 * Implementation for IClauseFilters method
 */
std::unique_ptr<ClauseFilter> createAndFilter(std::unique_ptr<ClauseFilter> left, std::unique_ptr<ClauseFilter> right) {
  return std::unique_ptr<ClauseFilter>(new AndFilter(std::move(left), std::move(right)));
}


