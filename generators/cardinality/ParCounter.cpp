// *****************************************************************
// *  ParCounter.cpp                                               *
// *                                                               *
// *  Generator for CNF files encoding cardinality constraints of  *
// *  the form <=k(x_1,...,x_n).                                   *
// *  The generator is based on the parallel counter design by     *
// *  Muller and Preparata (JACM 22:195-201, 1975).                *
// *                                                               *
// *  (c) 2005 by Carsten Sinz, University of Tuebingen, Germany   *
// *****************************************************************

#include <iostream>
#include <vector>
#include <algorithm>
#include <iterator>
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include "Auxiliary.h"

using namespace std;


// ****************** circuit generation functions *****************

int nrHalfAdders = 0;  // number of half and full adders
int nrFullAdders = 0;  // needed for the parallel counter

int nrClauses    = 0;  // number of clauses used for the counter

bool generateEquivalences; // produce equivalences instead of implications


// Generate half adder computing 'a' + 'b', leaving the result
// in 'sum' and 'carry'. The resulting clauses are written to
// stdout in DIMACS CNF format.

void genHalfAdder(int a, int b, int sum, int carry)
{
  if(generateEquivalences) {

    printf("%d %d %d 0\n",  a, b,-sum);    // sum = a XOR b
    printf("%d %d %d 0\n",  a,-b, sum);
    printf("%d %d %d 0\n", -a, b, sum);
    printf("%d %d %d 0\n", -a,-b,-sum);

    printf("%d %d 0\n",     a,   -carry);  // carry = a AND b
    printf("%d %d 0\n",     b,   -carry);
    printf("%d %d %d 0\n", -a,-b, carry);

  }
  else {

    printf("%d %d %d 0\n",  a,-b, sum);    // optimized version, resulting
    printf("%d %d %d 0\n", -a, b, sum);    // in lower number of clauses;
    printf("%d %d %d 0\n", -a,-b, carry);  // might have disadvantages during
                                           // search
  }
  
  nrHalfAdders++;
  nrClauses += (generateEquivalences ? 7 : 3);
}


// Generate full adder computing 'a' + 'b' + 'c', leaving the
// result in 'sum' and 'carry'. The resulting clauses are
// written to stdout in DIMACS CNF format.

void genFullAdder(int a, int b, int c, int sum, int carry)
{
  if(generateEquivalences) {

    printf("%d %d %d %d 0\n",  a, b, c,-sum);  // sum = a XOR b XOR c
    printf("%d %d %d %d 0\n",  a, b,-c, sum);
    printf("%d %d %d %d 0\n",  a,-b, c, sum);  // alternative encoding:
    printf("%d %d %d %d 0\n",  a,-b,-c,-sum);  // h = a XOR b
    printf("%d %d %d %d 0\n", -a, b, c, sum);  // sum = h XOR c
    printf("%d %d %d %d 0\n", -a, b,-c,-sum);  // (using one additional variable
    printf("%d %d %d %d 0\n", -a,-b, c,-sum);  //  and also eight 3-clauses)
    printf("%d %d %d %d 0\n", -a,-b,-c, sum);

    printf("%d %d %d 0\n",  a, b,-carry);      // carry = (a AND b) OR
    printf("%d %d %d 0\n",  a, c,-carry);      //              (c AND (a XOR b))
    printf("%d %d %d 0\n",  b, c,-carry);      // alternatively:
    printf("%d %d %d 0\n", -a,-b, carry);      // carry = (a AND b) OR (c AND h)
    printf("%d %d %d 0\n", -a,-c, carry);      // (h defined as above)
    printf("%d %d %d 0\n", -b,-c, carry);

  }
  else {

    printf("%d %d %d %d 0\n",  a, b,-c, sum);
    printf("%d %d %d %d 0\n",  a,-b, c, sum);  // alternative encoding:
    printf("%d %d %d %d 0\n", -a, b, c, sum);  // sum = h XOR c
    printf("%d %d %d %d 0\n", -a,-b,-c, sum);
    
    printf("%d %d %d 0\n", -a,-b, carry);      // carry = (a AND b) OR (c AND h)
    printf("%d %d %d 0\n", -a,-c, carry);      // (h as defined above)
    printf("%d %d %d 0\n", -b,-c, carry);

  }
 
  nrFullAdders++;
  nrClauses += (generateEquivalences ? 14 : 7);
}


// Generates the parallel counter circuit of Muller&Preparata (1975)
// inputs:  vector of indices of input bits to be counted (x_1,...,x_n)
// outputs: vector of indices of output bits (counter's result), i.e.
//          the number of input bits set to 1 in binary: d_m,...,d_0
//          NOTES: 1. The i-th bit is stored at index i, i.e. the LSB is
//                    in the first cell of the vector
//                 2. Vector 'outputs' has to be empty on entering the
//                    function; it is filled by the function

void genParCounter(vector<int> inputs, vector<int>& outputs)
{
  int n = inputs.size();
  int m = ilog2(inputs.size());

  // trivial case: only one input
  if(n == 1) {
    outputs.push_back(inputs[0]); // output is x_n
    return;
  }

  // split input bits into two halves: 0 to (p_end-1) and p_end to n-2
  // (input n-1 is treated separately)

  int p_end = ipow2(m)-1;
  
  vector<int> a_inputs, b_inputs, a_outputs, b_outputs;

  for(int i = 0; i < p_end; i++)
    a_inputs.push_back(inputs[i]);
  for(int i = p_end; i < n-1; i++)
    b_inputs.push_back(inputs[i]);

  genParCounter(a_inputs, a_outputs);
  if(b_inputs.size() > 0)
    genParCounter(b_inputs, b_outputs);

  int m_min = min(a_outputs.size(), b_outputs.size());

  // write full adders

  int carry = inputs[n-1]; // input bit (n-1) is the initial carry

  for(int i = 0; i < m_min; i++) {
    int sum = nextVarIndex();
    int nextCarry = nextVarIndex();
    genFullAdder(a_outputs[i], b_outputs[i], carry, sum, nextCarry);
    outputs.push_back(sum);
    carry = nextCarry;
  }

  // write half adders

  for(unsigned int i = m_min; i < a_outputs.size(); i++) {
    int sum = nextVarIndex();
    int nextCarry = nextVarIndex();
    genHalfAdder(a_outputs[i], carry, sum, nextCarry);
    outputs.push_back(sum);
    carry = nextCarry;
  }

  outputs.push_back(carry);
}


// Generate circuit computing 'lessThan' predicate.
// This predicate is true iff the binary number represented by the
// 'outputs' d_0, ..., d_m (LSB is first in vector) is less than or 
// equal to the integer 'k'.
// PRECONDITION: 'outputs'-vector must contain at least one element

void genLessThanCircuit(int k, vector<int> outputs)
{
  typedef vector<int> clause;
  typedef vector<clause> clause_set;
  clause_set cs;

  // produce clause set representing constraint on the
  // counter's output bits

  if(!(k & 1)) { // initialization: bit 0
    clause c;
    c.push_back(-outputs[0]);
    cs.push_back(c);
  }
  k >>= 1; // 'delete' bit 0
  for(unsigned int i=1; i < outputs.size(); i++) { // further bits
    char bit_i = k & 1; // get LSB
    if(bit_i)
      // add -d_i to all clauses in clause set
      for(clause_set::iterator j = cs.begin(); j != cs.end(); j++)
	j->push_back(-outputs[i]);
    else {
      clause c;
      // add new clause -d_i
      c.push_back(-outputs[i]);
      cs.push_back(c);
    }
    k >>= 1; // switch to next bit
  }

  // write clause set
  for(clause_set::iterator ci = cs.begin(); ci != cs.end(); ci++) {
    copy(ci->begin(), ci->end(), ostream_iterator<int>(cout, " "));
    cout << "0" << endl;
  }
  nrClauses += cs.size();
}


void lessThanConstraint(int k, vector<int> inputs)
{
  vector<int> outputs;
  genParCounter(inputs, outputs); // generate CNF of counter circuit
  genLessThanCircuit(k, outputs); // generate CNF of comparator circuit
}


int main(int argc, char* argv[])
{
  // parse command line: check for optional '-e'
  generateEquivalences = false;
  for(int p=1; p < argc; p++)
    if(strncmp("-e", argv[p], 2) == 0) {
      generateEquivalences = true;
      for(int j=p; j < argc-1; j++)
	argv[j] = argv[j+1];  // 'delete' argument
    }

  if(argc <= 2) {
    cerr << "usage: " << argv[0] << "[-e] <k> <n>" << endl;
    cerr << "   Generate clause set representing cardinality constraint "
	<< "<=k(x_1,...,x_n)." << endl;
    cerr << "   -e: use equivalences instead of implications (results in more" << endl
         << "       clauses but possibly better search behavior)" << endl;
    exit(10);
  }

  int k = atoi(argv[1]);
  int n = atoi(argv[2]);

  if(n < 1 || k < 0 || k > n) {
    cerr << "Command line parameter(s) ('k' or 'n') out of range!" << endl;
    exit(10);
  }

  cerr << "Generating cardinality constraint <=k(x_1,...,x_n) for "
       << "k = " << k << " and n = " << n << "." << endl;
  cerr << "Input variables have indices { 1, ..., " << n << " }." << endl;

  cout << "c Cardinality constraint <=k(x_1,...,x_n) for k = " << k << " and n = "
       << n << "." << endl;
  cout << "c Adders are encoded using "
       << (generateEquivalences ? "equivalences" : "implications") << "." << endl;
  cout << "c The numbers of clauses and variables on the next line are just upper"
       << " bounds!" << endl;
  cout << "p cnf " << 3*n - 2 << " " 
       << (generateEquivalences ? 14*n - 7*ilog2(n) - 14 : 7*n - 3*ilog2(n) - 7)
       << endl;

  vector<int> inputs;
  for(int i=1; i <= n; i++)
    inputs.push_back(nextVarIndex());
  lessThanConstraint(k, inputs);

  cerr << "Done. Circuit uses " << nrHalfAdders << " half adder(s) and "
       << nrFullAdders << " full adder(s)." << endl;
  cerr << "CNF representation contains " << nrClauses << " clause(s)." << endl;

  return 0;
}
