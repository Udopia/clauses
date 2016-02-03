// *****************************************************************
// *  SeqCounter.cpp                                               *
// *                                                               *
// *  Generator for CNF files encoding cardinality constraints of  *
// *  the form <=k(x_1,...,x_n).                                   *
// *  The generator is based on a sequential unary counter design. *
// *                                                               *
// *  (c) 2005 by Carsten Sinz, University of Tuebingen, Germany   *
// *****************************************************************

#include <iostream>
#include <vector>
#include <algorithm>
#include <iterator>
#include <stdio.h>
#include "Auxiliary.h"

using namespace std;


// ****************** circuit generation functions *****************

int nrClauses    = 0;      // number of clauses used for the counter

int s_index(int i, int j, int k)
{
  // auxiliary encoding variables s_{i,j}:
  // (n-1)*k variables, starting from 'firstEncVar'
  // index of s_{i,j} is: firstEncVar + i*k + j
  // (0 <= i < n-1, 0 <= j < k)

  static int firstEncVar = nextVarIndex();

  return firstEncVar + i*k + j;
}


// Generates sequential unary counter circuit with overflow detection.
// k:       limit (# of variables in inputs that may be simultaneously
//          set to true)
// inputs:  vector of indices of input variables

void genSeqCircuit(int k, vector<int> inputs)
{
  int n = inputs.size();
  
  // write clauses of first partial sum (i.e. i=0)
  printf("%d %d 0\n", -inputs[0], s_index(0,0,k));
  for(int j=1; j < k; j++)
    printf("%d 0\n", -s_index(0,j,k));
  
  // write clauses for general case (i.e. 0 < i < n-1)
  for(int i=1; i < n-1; i++) {
    printf("%d %d 0\n", -inputs[i], s_index(i,0,k));
    printf("%d %d 0\n", -s_index(i-1,0,k), s_index(i,0,k));
    for(int j=1; j < k; j++) {
      printf("%d %d %d 0\n", -inputs[i], -s_index(i-1,j-1,k), s_index(i,j,k));
      printf("%d %d 0\n", -s_index(i-1,j,k), s_index(i,j,k));
    }
    printf("%d %d 0\n", -inputs[i], -s_index(i-1,k-1,k));
  }
  
  // last clause for last variable
  printf("%d %d 0\n", -inputs[n-1], -s_index(n-2,k-1,k));

  nrClauses += 2*n*k + n - 3*k - 1;
}

void lessThanConstraint(int k, vector<int> inputs)
{
  genSeqCircuit(k, inputs);
}


int main(int argc, char* argv[])
{
  if(argc <= 2) {
    cerr << "usage: " << argv[0] << " <k> <n>" << endl;
    cerr << "   Generate clause set representing cardinality constraint "
	<< "<=k(x_1,...,x_n)." << endl;
    exit(10);
  }

  int k = atoi(argv[1]);
  int n = atoi(argv[2]);

  if(n < 2 || k < 1 || k > n) {
    cerr << "Command line parameter(s) ('k' or 'n') out of range!" << endl;
    exit(10);
  }

  cerr << "Generating cardinality constraint <=k(x_1,...,x_n) for "
       << "k = " << k << " and n = " << n << "." << endl;
  cerr << "Input variables have indices { 1, ..., " << n << " }." << endl;

  cout << "c Cardinality constraint <=k(x_1,...,x_n) for k = " << k << " and n = "
       << n << "." << endl;
  cout << "p cnf " << (n-1)*k+n << " " << 2*n*k + n - 3*k - 1 << endl;

  vector<int> inputs;
  for(int i=1; i <= n; i++)
    inputs.push_back(nextVarIndex());
  lessThanConstraint(k, inputs);

  cerr << "Done. CNF representation contains " << nrClauses << " clauses." << endl;

  return 0;
}
