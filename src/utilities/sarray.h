#pragma once

#include <iostream>


/*
    ---------- Stable array ----------

    This file contains a template definition of so called "stable array".
    - Stable array is a fixed size container with vector-like interface
    - It's based on static array and imitates dynamic size up until fixed upper bound
    - Similarly to stable stack, the memory is never released up until the final destruction of object
    - WARNING: There are no checks for exceeding max size of a container
*/

template <typename T, std::size_t MSize>
class StableArray
{
public:
    // Vector-like interface - adding & removing elements
    void push_back(const T& element) { *m_end = element; m_end++; }
    void pop_back() { m_end--; }
    void clear() { m_end = m_data; }
    void set_end(T* end) { m_end = end; }       // This is an anti-pattern, but might be necessary in some use cases

    // Vector-like interface - array indexing
    T& operator[](int i) { return m_data[i]; }
	const T& operator[](int i) const { return m_data[i]; }

    // Vector-like interface - size getters
    std::size_t size() const { return m_end - m_data; }
    bool empty() const { return m_end == m_data; }

    // STL compatibility - iterators
    T* begin() { return m_data; }
	const T* begin() const { return m_data; }
	T* end() { return m_end; }
	const T* end() const { return m_end; }

    // Printing
    friend std::ostream& operator<<(std::ostream& os, const StableArray<T, MSize>& array)
    {
        for (const T& element : array)
            os << element << "\n";
    
        return os;
    }

private:
    // Data container - static array
    T m_data[MSize];

    // Range pointer - determines the logical end of array
    T* m_end = m_data;
};