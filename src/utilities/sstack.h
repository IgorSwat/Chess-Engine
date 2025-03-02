#pragma once

#include <algorithm>
#include <vector>


/*
    ---------- Stable stack ----------

    This file contains a template definition of so called "stable stack".
    - Stable stack is a stack with no memory release up until the final destruction of an object
    - Removing an element from the stack only results in decrementing the element counter, but does not free memory
    - Adding an element only increments the element counter, unless container is already full - only then
      we create new element object instead of reusing the one from memory
    - Dynamic size achieved by using std::vector
    - NOTE: elements stored on stable stack must have a default constructor available
*/

template <typename T>
class StableStack
{
public:
    // Initialize container with some basic capacity
    StableStack() : data(32) {}

    // Stack interface - state modifiers
    void push() { current++; if (current == data.size()) data.emplace_back(); }
    void pop() { current = std::max(current - 1, -1); }
    void shrink() { current = 0; }      // Reduces (logically) the stack to only starting element
    void clear() { current = -1; }      // Completely clears (logically) the stack

    // Stack interface - getters
    T& top() { return data[current]; }                              // Equivalent of top(0)
    const T& top() const { return data[current]; }                  // Equivalent of top(0)
    T& top_n(int n) { return data[current - n]; }                   // n-th element from the top
    const T& top_n(int n) const { return data[current - n]; }       // n-th element from the top
    size_t size() const { return size_t(current + 1); }
    bool empty() const { return size() == -1; }

private:
    // Data container
    // - Array representation with std::vector is on average a little bit more efficient than double linked list representation
    // - Since std::vector automatically resizes itself, we can use stack of any size
    std::vector<T> data;

    // Pointer - vector index
    // - We assume that element with index -1 acts as null element (empty stack)
    int current = -1;
};