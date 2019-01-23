#include "TimeUtils.h"

TimeoPoint now() {
	return std::chrono::steady_clock::now();
}

float secondsBetween(TimeoPoint a, TimeoPoint b) {
	return std::chrono::duration<float>(b - a).count();
}
