#include "Auxiliary.h"

// ********************** auxiliary functions **********************

// Compute logarithm to base 2 of an integer n,
// i.e. log2(n); the result is rounded down
// PRECONDITION: n > 0

long ilog2(long n)
{
  int r = -1;
  while(n > 0) {
    n >>= 1;
    r++;
  }
  return r;
}

// Compute n-th power of 2, i.e. 2^n
// PRECONDITION: n >= 0

long ipow2(long n)
{
  long nHalf = n >> 1, power2_nHalf;
  power2_nHalf = (nHalf == 0 ? 1 : ipow2(nHalf));
  if(n % 2 == 0)
    return power2_nHalf * power2_nHalf;
  else
    return power2_nHalf * power2_nHalf * 2;
}

// Generate next variable index (starting from one, incrementing
// by one with each call)

int nextVarIndex(void)
{
  static int nextVar = 1;
  return nextVar++;
}
