#ifndef TIMEUTILS_H
#define TIMEUTILS_H

#include <chrono>

typedef std::chrono::steady_clock::time_point TimeoPoint;

TimeoPoint now();

float secondsBetween(TimeoPoint a, TimeoPoint b);

#endif
