#ifndef COLLECTIONUTILS_H
#define COLLECTIONUTILS_H

#include <set>
#include <vector>

template<typename I, typename O> std::set<O> asSet(std::vector<I> values) {
	return std::set<O>(values.begin(), values.end());
}

template<typename T> std::set<T> asSet(std::vector<T> values) {
	return asSet<T,T>(values);
}

#endif
