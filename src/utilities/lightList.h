#pragma once

#include <iostream>


// ------------------
// LightList template
// ------------------

// A lightweight, vector-like adapter for C-style array used to iterate over given (dynamic) set of elements
template <typename T, std::size_t Msize = 256>
class LightList
{
public:
	// STL algorithms compatibility
	T* begin() { return data; }
	const T* begin() const { return data; }
	T* end() { return endPtr; }
	const T* end() const { return endPtr; }

	T& operator[](int i) { return data[i]; }
	const T& operator[](int i) const { return data[i]; }

	void push_back(const T& move) { *endPtr = move; endPtr++; }
	void pop_back() { endPtr--; }
	void clear() { endPtr = data; }	// Does not remove elements directly, just manipulates with range pointers
	void setEnd(T* end) { endPtr = end; }
	std::size_t size() const { return endPtr - data; }

	friend std::ostream& operator<<(std::ostream& os, const LightList<T, Msize>& list);

private:
	T data[Msize] = {};
	T* endPtr = data;
};


// -------------
// Miscellaneous
// -------------

template <typename T, std::size_t Msize>
std::ostream& operator<<(std::ostream& os, const LightList<T, Msize>& list)
{
    for (const T& element : list)
        os << element << "\n";
    
    return os;
}