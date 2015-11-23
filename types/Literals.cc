/*
 * Literals.cc
 *
 *  Created on: 28.05.2014
 *      Author: markus
 */

#include "Literals.h"
#include "ClauseList.h"

namespace Dark {

Literals::Literals() {
  literals = new std::vector<Literal>();
  mark = false;
}

Literals::Literals(Literal lit) {
  literals = new std::vector<Literal>();
  this->add(lit);
  mark = false;
}

Literals::Literals(Literal lit1, Literal lit2) {
  literals = new std::vector<Literal>();
  this->add(lit1);
  this->add(lit2);
  mark = false;
}

Literals::Literals(Literal lit1, Literal lit2, Literal lit3) {
  literals = new std::vector<Literal>();
  this->add(lit1);
  this->add(lit2);
  this->add(lit3);
  mark = false;
}

Literals::Literals(LiteralList* lits) {
  literals = lits;
  mark = false;
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
  if (max_var < var(lit)) max_var = var(lit);
  literals->push_back(lit);
}

void Literals::addAll(Literals* clause) {
  if (max_var < clause->maxVar()) max_var = clause->maxVar();
  literals->insert(literals->end(), clause->begin(), clause->end());
}

void Literals::addAll(std::vector<Literal>* clause) {
  for (std::vector<Literal>::iterator it = literals->begin(); it != literals->end(); it++) {
    this->add(*it);
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

Literal& Literals::operator[] (const int i) {
    return (*literals)[i];
}

int Literals::pos(Literal literal) {
  for (unsigned int i = 0; i < literals->size(); i++) {
    if ((*literals)[i] == literal) {
      return i;
    }
  }
  return -1;
}

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

// from clause
void Literals::setMarked() {
  mark = true;
}

void Literals::unsetMarked() {
  mark = false;
}

bool Literals::isMarked() {
  return mark;
}

void Literals::inlineNegate() {
  for (unsigned int i = 0; i < size(); i++) {
    (*literals)[i] = ~(*literals)[i];
  }
}

bool Literals::isBlockedBy(Literal blocking, Literals* clause) {
  for (Literals::iterator it = clause->begin(); it != clause->end(); ++it) {
    Literal& lit = *it;
    if (lit != ~blocking && this->contains(~lit)) {
      return true;
    }
  }
  return false;
}

// from cube
Literals* Literals::negate() {
  Literals* lits = new Literals();
  for (iterator it = begin(); it != end(); ++it) {
    lits->add(~(*it));
  }
  return lits;
}

void Literals::clear() {
  literals->clear();
}

Literals* Literals::clone() {
  Literals* lits = new Literals();
  lits->addAll(this);
  return lits;
}

bool Literals::isConsistentWith(Literals* lits) {
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

int Literals::cardinality(Literals* lits) {
  int count = 0;
  for (unsigned int j = 0; j < lits->size(); j++) {
    if (this->contains(lits->get(j))) {
      count++;
    }
  }
  return count;
}

bool Literals::satisfies(Literals* lits) {
  for (unsigned int j = 0; j < lits->size(); j++) {
    if (this->contains(lits->get(j))) {
      return true;
    }
  }
  return false;
}

bool Literals::falsifies(Literals* lits) {
  for (unsigned int j = 0; j < lits->size(); j++) {
    if (!this->contains(~(lits->get(j)))) {
      return false;
    }
  }
  return true;
}

ClauseList* Literals::checkSatisfied(ClauseList* list) {
  ClauseList* notSatisfied = new ClauseList();
  for (unsigned int i = 0; i < list->size(); i++) {
    Literals* lits = list->get(i);
    if (!this->satisfies(lits)) {
      notSatisfied->add(lits);
    }
  }
  return notSatisfied;
}

}
