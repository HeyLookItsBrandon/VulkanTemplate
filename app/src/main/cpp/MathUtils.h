#ifndef MATHUTILS_H
#define MATHUTILS_H

#include <algorithm>

template<typename T> T clamp(T value, T minimum, T maximum) {
	return std::max(minimum, std::min(value, maximum)) ;
}

#endif
