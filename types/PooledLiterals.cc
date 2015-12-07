/*
 * PooledLiterals.cc
 *
 *  Created on: Jul 29, 2015
 *      Author: markus
 */

#include "PooledLiterals.h"
#include "LiteralPool.h"
#include <cstring>

namespace Dark {

LiteralPool* PooledLiterals::pool = new LiteralPool();

PooledLiterals::PooledLiterals() {
  max_var = 0;
  nlits = 0;
  mark = false;
  watcher[0] = litFalse;
  watcher[1] = litFalse;
  literals = NULL;
}
PooledLiterals::PooledLiterals(int count) {
  max_var = 0;
  nlits = count;
  mark = false;
  watcher[0] = litFalse;
  watcher[1] = litFalse;
  literals = pool->alloc(count);
}
PooledLiterals::PooledLiterals(Literal lit) {
  max_var = var(lit);
  nlits = 1;
  mark = false;
  watcher[0] = lit;
  watcher[1] = litFalse;
  literals = NULL;
}
PooledLiterals::PooledLiterals(Literal lit1, Literal lit2) {
  max_var = std::max(var(lit1), var(lit2));
  nlits = 2;
  mark = false;
  watcher[0] = lit1;
  watcher[1] = lit2;
  literals = NULL;
}
// lits->size > 1
PooledLiterals::PooledLiterals(Literal lits[]) {
  max_var = 0;
  nlits = 0;
  for (int i = 0; lits[i] != litFalse; i++) {
    nlits++;
    if (var(lits[i]) > max_var) max_var = var(lits[i]);
  }
  mark = false;
  watcher[0] = lits[0];
  watcher[1] = lits[1];
  literals = pool->alloc(lits);
}

PooledLiterals::~PooledLiterals() { }

int PooledLiterals::maxVar() {
  return max_var;
}

PooledLiterals::iterator PooledLiterals::begin() {
  return literals != NULL ? pool->resolve(literals) : watcher;
}

PooledLiterals::iterator PooledLiterals::end() {
  return literals != NULL ? pool->resolve((Literal*)(literals + nlits)) : watcher + nlits;
}

void PooledLiterals::add(Literal lit) {
  *end() = lit;
  Literal* lits = pool->alloc(literals);
  pool->free(literals);
  literals = lits;
}

void PooledLiterals::addAll(PooledLiterals* clause) {
  Literal* lits = pool->alloc(nlits + clause->nlits);
  memcpy(lits, begin(), nlits);
  memcpy(lits + nlits, clause->begin(), clause->nlits);
  pool->free(literals);
  literals = lits;
}

bool PooledLiterals::remove(Literal lit) {
  for (iterator it = begin(); *it != litFalse; it++) {
    if (*it == lit) {
      *it = litFalse;
      std::swap(*it, *(end()-1));
      return true;
    }
  }
  return false;
}

Literal PooledLiterals::removeLast() {
  iterator it = end()-1;
  Literal last = *it;
  *it = litFalse;
  return last;
}

void PooledLiterals::removeAll(PooledLiterals* lits) {
  for (Literal lit : *lits) {
    remove(lit);
  }
}

void PooledLiterals::sort() {
  std::sort(begin(), end());
}

PooledLiterals* PooledLiterals::slice(int from) {
  return new PooledLiterals(begin() + from);
}

PooledLiterals* PooledLiterals::slice(int from, int to) {
  std::swap(*end(), *(begin()+to));
  PooledLiterals* result = new PooledLiterals(begin()+from);
  std::swap(*end(), *(begin()+to));
  return result;
}

Literal PooledLiterals::get(int i) {
  return *(begin()+i);
}

unsigned int PooledLiterals::size() {
  return nlits;
}

Literal PooledLiterals::operator[] (const int i) {
  return get(i);
}

int PooledLiterals::pos(Literal literal) {
  for (iterator it = begin(); *it != litFalse; it++) {
    if (*it == literal) return it - begin();
  }
  return -1;
}

bool PooledLiterals::contains(Literal literal) {
  for (iterator it = begin(); *it != litFalse; it++) {
    if (*it == literal) return true;
  }
  return false;
}

bool PooledLiterals::entails(PooledLiterals* clause) {
  if (size() < clause->size()) return false;
  for (iterator it = clause->begin(); *it != litFalse; it++) {
    if (!this->contains(*it)) return false;
  }
  return true;
}

bool PooledLiterals::equals(PooledLiterals* clause) {
  if (size() != clause->size()) return false;
  return this->entails(clause) && clause->entails(this);
}

void PooledLiterals::print(FILE* out) {
  for (iterator it = begin(); *it != litFalse; it++) {
    if (it != begin()) fprintf(out, ", ");
    fprintf(out, "%s%i", sign(*it)?"-":"", var(*it));
  }
}

void PooledLiterals::println(FILE* out) {
  print(out);
  fprintf(out, "\n");
}

void PooledLiterals::printDimacs(FILE* out) {
  for (iterator it = begin(); *it != litFalse; it++) {
    fprintf(out, "%s%i", sign(*it)?"-":"", var(*it));
    fprintf(out, " ");
  }
  fprintf(out, "0\n");
}

std::string* PooledLiterals::toString() {
  char buffer[3*nlits];
  for (iterator it = begin(); *it != litFalse; it++) {
    if (it != begin()) sprintf(buffer, ", ");
    sprintf(buffer, "%s%i", sign(*it)?"-":"", var(*it));
  }
  return new std::string(buffer);
}

PooledLiterals* PooledLiterals::allBut(Literal lit) {
  PooledLiterals* lits = new PooledLiterals(nlits-1);
  iterator src = begin();
  for (iterator dest = lits->begin(); dest != lits->end(); dest++, src++) {
    if (*src != lit) *dest = *src;
  }
  watcher[0] = *(lits->begin());
  watcher[1] = *(lits->begin()+1);
  return lits;
}

// from clause
void PooledLiterals::setMarked() { mark = true; }
void PooledLiterals::unsetMarked() { mark = false; }
bool PooledLiterals::isMarked() { return mark; }

void PooledLiterals::inlineNegate() {
  for (Literal* lit = pool->resolve(literals); *lit != litFalse; lit++) {
    *lit = ~(*lit);
  }
}

bool PooledLiterals::isBlockedBy(Literal blocking, PooledLiterals* clause) {
  for (Literal* lit = pool->resolve(literals); *lit != litFalse; lit++) {
    if (*lit == ~blocking) return true;
  }
  return false;
}

// from cube
PooledLiterals* PooledLiterals::negate() {
  PooledLiterals* lits = new PooledLiterals(pool->resolve(literals));
  lits->inlineNegate();
  return lits;
}

void PooledLiterals::clear() {
  memset(begin(), 0, nlits);
  memset(watcher, 0, 2);
}

bool PooledLiterals::isConsistentWith(PooledLiterals* model) {
  for (iterator it = begin(); *it != litFalse; it++) {
    if (model->contains(~(*it))) return false;
  }
  return true;
}

int PooledLiterals::cardinality(PooledLiterals* model) {
  int result = 0;
  for (iterator it = begin(); *it != litFalse; it++) {
    if (model->contains(*it)) result++;
  }
  return result;
}

bool PooledLiterals::satisfies(PooledLiterals* model) {
  for (iterator it = begin(); *it != litFalse; it++) {
    if (model->contains(*it)) return true;
  }
  return false;
}

bool PooledLiterals::falsifies(PooledLiterals* model) {
  for (iterator it = begin(); *it != litFalse; it++) {
    if (!model->contains(~(*it))) return false;
  }
  return true;
}

} /* namespace Dark */
