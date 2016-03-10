/*
 * Literals.cc
 *
 *  Created on: 28.05.2014
 *      Author: markus
 */

#include "ClauseList.h"
#include "DynamicLiterals.h"

namespace Dark {

DynamicLiterals::DynamicLiterals() {
  literals = new std::vector<Literal>();
  mark = false;
}

DynamicLiterals::DynamicLiterals(Literal lit) {
  literals = new std::vector<Literal>();
  this->add(lit);
  mark = false;
}

DynamicLiterals::DynamicLiterals(Literal lit1, Literal lit2) {
  literals = new std::vector<Literal>();
  this->add(lit1);
  this->add(lit2);
  mark = false;
}

DynamicLiterals::DynamicLiterals(Literal lit1, Literal lit2, Literal lit3) {
  literals = new std::vector<Literal>();
  this->add(lit1);
  this->add(lit2);
  this->add(lit3);
  mark = false;
}

DynamicLiterals::DynamicLiterals(LiteralList* lits) {
  literals = lits;
  mark = false;
}

DynamicLiterals::~DynamicLiterals() {
  delete literals;
}

/**
 * Manipulators
 */

void DynamicLiterals::add(Literal lit) {
  if (max_var < var(lit)) max_var = var(lit);
  literals->push_back(lit);
}

void DynamicLiterals::addAll(DynamicLiterals* clause) {
  if (max_var < clause->maxVar()) max_var = clause->maxVar();
  literals->insert(literals->end(), clause->begin(), clause->end());
}

void DynamicLiterals::addAll(std::vector<Literal>* clause) {
  for (std::vector<Literal>::iterator it = literals->begin(); it != literals->end(); it++) {
    this->add(*it);
  }
}


Literal DynamicLiterals::removeLast() {
  Literal lit = this->getLast();
  literals->pop_back();
  return lit;
}

bool DynamicLiterals::remove(Literal literal) {
  for (std::vector<Literal>::iterator it = literals->begin(); it != literals->end(); it++) {
    if (literal == *it) {
      literals->erase(it);
      return true;
    }
  }
  return false;
}

void DynamicLiterals::removeAll(DynamicLiterals* clause) {
  for (std::vector<Literal>::iterator it = clause->begin(); it != clause->end(); it++) {
    remove(*it);
  }
}

void DynamicLiterals::sort() {
  std::sort(literals->begin(), literals->end());
}

DynamicLiterals* DynamicLiterals::slice(int start) {
  return this->slice(start, literals->size());
}

/*
 * returns list in interval [start, end)
 */
DynamicLiterals* DynamicLiterals::slice(int start, int end) {
  std::vector<Literal>* result = new std::vector<Literal>();
  int n = end - start;
  for (int i = 0; i < n; i++) {
    result->push_back(this->get(start + i));
  }
  return new DynamicLiterals(result);
}

/**
 * Accessors
 */
DynamicLiterals::iterator DynamicLiterals::begin() {
  return literals->begin();
}

DynamicLiterals::iterator DynamicLiterals::end() {
  return literals->end();
}

Literal DynamicLiterals::getFirst() {
  return *(literals->begin());
}

Literal DynamicLiterals::getLast() {
  return *(literals->rbegin());
}

Literal DynamicLiterals::get(int i) {
  return (*literals)[i];
}

unsigned int DynamicLiterals::size() {
  return literals->size();
}

Literal& DynamicLiterals::operator[] (const int i) {
    return (*literals)[i];
}

int DynamicLiterals::pos(Literal literal) {
  for (unsigned int i = 0; i < literals->size(); i++) {
    if ((*literals)[i] == literal) {
      return i;
    }
  }
  return -1;
}

bool DynamicLiterals::contains(Literal literal) {
  return std::find(literals->begin(), literals->end(), literal) != literals->end();
}

bool DynamicLiterals::entails(DynamicLiterals* clause) {
  for (iterator it = literals->begin(); it != literals->end(); ++it) {
    if (!clause->contains(*it)) {
      return false;
    }
  }
  return true;
}

bool DynamicLiterals::equals(DynamicLiterals* clause) {
  return this->entails(clause) && clause->entails(this);
}

/**
 * Output
 */
void DynamicLiterals::print(FILE* out) {
  fprintf(out, "(");
  for (iterator it = literals->begin(); it != literals->end(); ++it) {
    fprintf(out, "%s%s%i", it != literals->begin() ? "," : "", sign(*it) ? "-" : "", var(*it));
  }
  fprintf(out, ")");
}

void DynamicLiterals::println(FILE* out) {
  print(out);
  fprintf(out, "\n");
}

void DynamicLiterals::printDimacs(FILE* out) {
  for (iterator it = literals->begin(); it != literals->end(); ++it) {
    fprintf(out, "%s%i ", sign(*it) ? "-" : "", var(*it));
  }
  fprintf(out, "0\n");
}

std::string* DynamicLiterals::toString() {
  std::string* val = new std::string();
  val->append("(");
  for (iterator it = literals->begin(); it != literals->end(); ++it) {
    char buf[256];
    sprintf(buf, "%s%s%i", it != literals->begin() ? "," : "", sign(*it) ? "-" : "", var(*it));
    val->append(buf);
  }
  val->append(")");
  return val;
}

DynamicLiterals* DynamicLiterals::allBut(Literal exclude) {
  std::vector<Literal>* vec = new std::vector<Literal>();
  for (LiteralList::iterator it = literals->begin(); it != literals->end(); ++it) {
    Literal lit = *it;
    if (lit != exclude) {
      vec->push_back(lit);
    }
  }
  return new DynamicLiterals(vec);
}

// from clause
void DynamicLiterals::setMarked() {
  mark = true;
}

void DynamicLiterals::unsetMarked() {
  mark = false;
}

bool DynamicLiterals::isMarked() {
  return mark;
}

void DynamicLiterals::inlineNegate() {
  for (unsigned int i = 0; i < size(); i++) {
    (*literals)[i] = ~(*literals)[i];
  }
}

bool DynamicLiterals::isBlockedBy(Literal blocking, DynamicLiterals* clause) {
  for (DynamicLiterals::iterator it = clause->begin(); it != clause->end(); ++it) {
    Literal& lit = *it;
    if (lit != ~blocking && this->contains(~lit)) {
      return true;
    }
  }
  return false;
}

// from cube
DynamicLiterals* DynamicLiterals::negate() {
  DynamicLiterals* lits = new DynamicLiterals();
  for (iterator it = begin(); it != end(); ++it) {
    lits->add(~(*it));
  }
  return lits;
}

void DynamicLiterals::clear() {
  literals->clear();
}

DynamicLiterals* DynamicLiterals::clone() {
  DynamicLiterals* lits = new DynamicLiterals();
  lits->addAll(this);
  return lits;
}

bool DynamicLiterals::isConsistentWith(DynamicLiterals* lits) {
  for (unsigned int i = 0, j = 0; i < this->size() && j < lits->size(); ) {
    Literal lit1 = this->get(i);
    Literal lit2 = lits->get(j);
    if (var(lit1) < var(lit2)) {
      i++;
    } else if (var(lit2) < var(lit1)) {
      j++;
    } else {
      if (lit1 == ~lit2) {
        return false;
      }
      i++;
      j++;
    }
  }
  return true;
}

}
