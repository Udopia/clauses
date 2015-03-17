/*
 * Literal.h
 *
 *  Created on: Jan 7, 2015
 *      Author: markus
 */

#ifndef LITERAL_H_
#define LITERAL_H_

#ifdef MINISAT_TYPES
  #include "minisat/core/SolverTypes.h"
  typedef Minisat::Var Var;
  typedef Minisat::Lit Literal;
//  using Var = Minisat::Var;
//  using Literal = Minisat::Lit;
  using namespace Minisat;
#else
  typedef int Var;

  struct Literal {
    int x;

    // Use this as a constructor:
    friend Literal mkLit(Var var, bool sign = false);

    bool operator == (Literal p) const { return x == p.x; }
    bool operator != (Literal p) const { return x != p.x; }
    bool operator < (Literal p) const { return x < p.x;  } // '<' makes p, ~p adjacent in the ordering.
  };


  inline Literal mkLit (Var var, bool sign) { Literal p; p.x = var + var + (int)sign; return p; }
  inline Literal operator ~(Literal p) { Literal q; q.x = p.x ^ 1; return q; }
  inline Literal operator ^(Literal p, bool b) { Literal q; q.x = p.x ^ (unsigned int)b; return q; }
  inline bool sign (Literal p) { return p.x & 1; }
  inline int var (Literal p) { return p.x >> 1; }

  // Mapping Literals to and from compact integers suitable for array indexing:
  inline int toInt (Var v) { return v; }
  inline int toInt (Literal p) { return p.x; }
  inline Literal toLit (int i) { Literal p; p.x = i; return p; }
#endif

#endif /* LITERAL_H_ */
