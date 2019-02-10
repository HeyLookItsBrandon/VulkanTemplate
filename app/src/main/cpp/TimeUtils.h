#ifndef TIMEUTILS_H
#define TIMEUTILS_H

#include <chrono>

typedef std::chrono::steady_clock::time_point TimePoint;

TimePoint now();

float secondsBetween(TimePoint a, TimePoint b);

#endif
