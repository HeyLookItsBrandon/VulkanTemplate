#include "TimeUtils.h"

TimePoint now() {
	return std::chrono::steady_clock::now();
}

float secondsBetween(TimePoint a, TimePoint b) {
	return std::chrono::duration<float>(b - a).count();
}
