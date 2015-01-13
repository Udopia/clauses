/*
 * Projection.cpp
 *
 *  Created on: 09.01.2014
 *      Author: markus
 */


#include "Projection.h"

#include "../types/Cube.h"


using namespace std;

namespace Dark {

Projection::Projection() {
  projection = new std::vector<pair<int,int>*>();
}

Projection::~Projection() {
  for (std::vector<pair<int,int>*>::iterator it = projection->begin(); it != projection->end(); ++it) {
    delete *it;
  }
  delete projection;
}

/**
 * Projection uses Variable-IDs (starting with 1)
 * and not Minisat-Variables (starting with 0)
 * !!!
 */
void Projection::addSequence(int from, int to) {
  pair<int,int>* s = new pair<int,int>();
  s->first = from;
  s->second = to;
  this->projection->push_back(s);
}

int Projection::size() {
  int num = 0;
  for (unsigned int i = 0; i < projection->size(); i++) {
    pair<int,int>* s = (*projection)[i];
    num += s->second - s->first + 1; // closed interval, thus +1
  }
  return num;
}

bool Projection::contains(int num) {
  for (unsigned int i = 0; i < projection->size(); i++) {
    pair<int,int>* s = (*projection)[i];
    if (s->first <= num && num <= s->second) {
      return true;
    }
  }
  return false;
}

Cube* Projection::project(Cube* cube) {
  Cube* result = new Cube();
  for (std::vector<Literal>::iterator it = cube->begin(); it != cube->end(); ++it) {
    Literal lit = *it;
    if (this->contains(var(lit)+1)) {
      result->add(lit);
    }
  }
  return result;
}

void Projection::println() {
  for (unsigned int i = 0; i < projection->size(); i++) {
    pair<int, int>* seq = (*projection)[i];
    printf("[%i,%i]; ", seq->first, seq->second);
  }
}

} /* namespace Analyzer */
