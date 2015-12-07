/*
 * IClauseFilter.h
 *
 *  Created on: Jun 2, 2014
 *      Author: markus
 */

#ifndef IFILTER_H_
#define IFILTER_H_

namespace Dark {
class DynamicLiterals;
}

class ClauseFilter {
public:
  virtual ~ClauseFilter() { };
  virtual bool meetCriteria(Dark::DynamicLiterals* clause) = 0;
};

#endif /* IFILTER_H_ */
