#ifndef __MyLossFunc__
#define __MyLossFunc__ 1

#include <cmath>

double DiffSquare(double*, double const*, double const*, size_t);
double CrossEntropy(double*, double const*, double const*, size_t);

#endif
static_assert(__MyLossFunc__ == 1, "Illegal header file!");
