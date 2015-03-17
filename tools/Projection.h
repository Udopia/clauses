/*
 * Projection.h
 *
 *  Created on: 09.01.2014
 *      Author: markus
 */

#ifndef TOOLS_PROJECTION_H_
#define TOOLS_PROJECTION_H_

#include <vector>
#include <utility>
#include <stdio.h>

namespace Dark {

class Literals;

class Projection {
public:
  Projection();
  Projection(const char* project);
  virtual ~Projection();

  void addSequence(int from, int to);
  int size();
  bool contains(int num);

  void println();

  /**
   * apply projection to given cube
   */
  Literals* project(Literals* cube);

private:
  /**
   * stores a compressed list of variables
   * to project on (subject to minimization, etc.)
   */
  std::vector<std::pair<int,int>*>* projection;
};

} /* namespace Analyzer */
#endif /* TOOLS_PROJECTION_H_ */
