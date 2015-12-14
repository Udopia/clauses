/*
 * IClauseFilter.h
 *
 *  Created on: Jun 2, 2014
 *      Author: markus
 */

#ifndef IFILTER_H_
#define IFILTER_H_

namespace Dark {
class PooledLiterals;
}

class ClauseFilter {
public:
  virtual ~ClauseFilter() { };
  virtual bool meetCriteria(Dark::PooledLiterals* clause) = 0;
};

#endif /* IFILTER_H_ */
