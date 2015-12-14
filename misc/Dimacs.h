/****************************************************************************************[Dimacs.h]
Copyright (c) 2003-2006, Niklas Een, Niklas Sorensson
Copyright (c) 2007-2010, Niklas Sorensson

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
associated documentation files (the "Software"), to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge, publish, distribute,
sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or
substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT
OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
**************************************************************************************************/

#ifndef Minisat_Dimacs_h
#define Minisat_Dimacs_h

#include <stdio.h>

#include "ParseUtils.h"
#include "../types/Literal.h"
#include "../types/ClauseList.h"
#include "../types/PooledLiterals.h"

//=================================================================================================
// DIMACS Parser:
class Dimacs {

public:
  Dimacs(gzFile input_stream, bool strictp = false) {
    declVars = 0;
    declClauses = 0;
    trueVars = 0;
    trueClauses = 0;
    this->clauses = new Dark::ClauseList();
    Minisat::StreamBuffer in(input_stream);
    parse(in);
  }

  Dark::ClauseList* getClauses() {
    return clauses;
  }

  unsigned int getDeclNVars() { return declVars; }
  unsigned int getDeclNClauses() { return declClauses; }
  unsigned int getRealNVars() { return trueVars; }
  unsigned int getRealNClauses() { return trueClauses; }

private:
  unsigned int declVars;
  unsigned int declClauses;
  unsigned int trueVars;
  unsigned int trueClauses;
  Literal* buffer;

  Dark::ClauseList* clauses;

  template<class B>
  Dark::PooledLiterals* readClause(B& in) {
    Literal* lit = buffer;
    int parsed_lit = parseInt(in);
    while (parsed_lit != 0) {
      int var = abs(parsed_lit);
      while (var >= trueVars) trueVars++;
      *lit = mkLit(var, parsed_lit < 0);
      lit++;
      parsed_lit = parseInt(in);
    }
    *lit = litFalse;
    return new Dark::PooledLiterals(buffer);
  }

  template<class B>
  void parse(B& in) {
    skipWhitespace(in);
    while (*in != EOF) {
      if (*in == 'p') {
        if (eagerMatch(in, "p cnf")){
          declVars    = parseInt(in);
          declClauses = parseInt(in);
          buffer = (Literal*)calloc(declVars+1, sizeof(Literal));
        } else {
          printf("PARSE ERROR! Unexpected char: %c\n", *in), exit(3);
        }
        skipLine(in);
      }
      else if (*in == 'c') {
        skipLine(in);
      }
      else {
        trueClauses++;
        clauses->add(readClause(in));
      }
      skipWhitespace(in);
    }
  }

};
#endif
