/*
 * Literals.cc
 *
 *  Created on: 28.05.2014
 *      Author: markus
 */

#include "Literals.h"

namespace Dark {

Literals::Literals() {
  literals = new std::vector<Literal>();
}

Literals::Literals(Literal lit) {
  literals = new std::vector<Literal>();
  this->add(lit);
}

Literals::Literals(Literal lit1, Literal lit2) {
  literals = new std::vector<Literal>();
  this->add(lit1);
  this->add(lit2);
}

Literals::Literals(Literal lit1, Literal lit2, Literal lit3) {
  literals = new std::vector<Literal>();
  this->add(lit1);
  this->add(lit2);
  this->add(lit3);
}

Literals::Literals(LiteralList* lits) {
  literals = lits;
}

Literals::~Literals() {
  delete literals;
}

void Literals::Init() {
  literals = new std::vector<Literal>();
}

/**
 * Manipulators
 */

void Literals::add(Literal lit) {
  literals->push_back(lit);
}

void Literals::addAll(Literals* clause) {
  for (std::vector<Literal>::iterator it = clause->begin(); it != clause->end(); it++) {
    add(*it);
  }
}

Literal Literals::removeLast() {
  Literal lit = this->getLast();
  literals->pop_back();
  return lit;
}

bool Literals::remove(Literal literal) {
  for (std::vector<Literal>::iterator it = literals->begin(); it != literals->end(); it++) {
    if (literal == *it) {
      literals->erase(it);
      return true;
    }
  }
  return false;
}

void Literals::removeAll(Literals* clause) {
  for (std::vector<Literal>::iterator it = clause->begin(); it != clause->end(); it++) {
    remove(*it);
  }
}

void Literals::sort() {
  std::sort(literals->begin(), literals->end());
}

Literals* Literals::slice(int start) {
  return this->slice(start, literals->size());
}

/*
 * returns list in interval [start, end)
 */
Literals* Literals::slice(int start, int end) {
  std::vector<Literal>* result = new std::vector<Literal>();
  int n = end - start;
  for (int i = 0; i < n; i++) {
    result->push_back(this->get(start + i));
  }
  return new Literals(result);
}

/**
 * Accessors
 */
Literals::iterator Literals::begin() {
  return literals->begin();
}

Literals::iterator Literals::end() {
  return literals->end();
}

Literal Literals::getFirst() {
  return *(literals->begin());
}

Literal Literals::getLast() {
  return *(literals->rbegin());
}

Literal Literals::get(int i) {
  return (*literals)[i];
}

unsigned int Literals::size() {
  return literals->size();
}

/**
 * Comparators
 */
bool Literals::contains(Literal literal) {
  return std::find(literals->begin(), literals->end(), literal) != literals->end();
}

bool Literals::entails(Literals* clause) {
  for (iterator it = literals->begin(); it != literals->end(); ++it) {
    if (!clause->contains(*it)) {
      return false;
    }
  }
  return true;
}

bool Literals::equals(Literals* clause) {
  return this->entails(clause) && clause->entails(this);
}

/**
 * Output
 */
void Literals::print(FILE* out) {
  fprintf(out, "(");
  for (iterator it = literals->begin(); it != literals->end(); ++it) {
    fprintf(out, "%s%s%i", it != literals->begin() ? "," : "", sign(*it) ? "-" : "", 1+var(*it));
  }
  fprintf(out, ")");
}

void Literals::println(FILE* out) {
  print(out);
  fprintf(out, "\n");
}

void Literals::printDimacs(FILE* out) {
  for (iterator it = literals->begin(); it != literals->end(); ++it) {
    fprintf(out, "%s%i ", sign(*it) ? "-" : "", 1+var(*it));
  }
  fprintf(out, "0\n");
}

std::string* Literals::toString() {
  std::string* val = new std::string();
  val->append("(");
  for (iterator it = literals->begin(); it != literals->end(); ++it) {
    char buf[256];
    sprintf(buf, "%s%s%i", it != literals->begin() ? "," : "", sign(*it) ? "-" : "", 1+var(*it));
    val->append(buf);
  }
  val->append(")");
  return val;
}

Literals* Literals::allBut(Literal exclude) {
  std::vector<Literal>* vec = new std::vector<Literal>();
  for (LiteralList::iterator it = literals->begin(); it != literals->end(); ++it) {
    Literal lit = *it;
    if (lit != exclude) {
      vec->push_back(lit);
    }
  }
  return new Literals(vec);
}

}
