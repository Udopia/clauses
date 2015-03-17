/*
 * IClauseFilter.h
 *
 *  Created on: Jun 2, 2014
 *      Author: markus
 */

#ifndef IFILTER_H_
#define IFILTER_H_

namespace Dark {
class Literals;
}

class ClauseFilter {
public:
  virtual ~ClauseFilter() { };
  virtual bool meetCriteria(Dark::Literals* clause) = 0;
};

#endif /* IFILTER_H_ */
