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
#include "types/Literal.h"
#include "types/Clause.h"
#include "types/ClauseList.h"

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

  int getDeclNVars() { return declVars; }
  int getDeclNClauses() { return declClauses; }
  int getRealNVars() { return trueVars; }
  int getRealNClauses() { return trueClauses; }

private:
  int declVars;
  int declClauses;
  int trueVars;
  int trueClauses;

  Dark::ClauseList* clauses;

  template<class B>
  Dark::Clause* readClause(B& in) {
    Dark::Clause* clause = new Dark::Clause();
    int parsed_lit = parseInt(in);
    while (parsed_lit != 0) {
      int var = abs(parsed_lit)-1;
      while (var >= trueVars) trueVars++;
      clause->add(mkLit(var, parsed_lit < 0));
      parsed_lit = parseInt(in);
    }
    return clause;
  }

  template<class B>
  void parse(B& in) {
    skipWhitespace(in);
    while (*in != EOF) {
      if (*in == 'p') {
        if (eagerMatch(in, "p cnf")){
          declVars    = parseInt(in);
          declClauses = parseInt(in);
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
