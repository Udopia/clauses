/*
 * IClauseFilter.h
 *
 *  Created on: Jun 2, 2014
 *      Author: markus
 */

#ifndef IFILTER_H_
#define IFILTER_H_

namespace Analyzer {
class Clause;
}

class ClauseFilter {
public:
  virtual ~ClauseFilter() { };
  virtual bool meetCriteria(Analyzer::Clause* clause) = 0;
};

#endif /* IFILTER_H_ */
