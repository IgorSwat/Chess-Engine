#pragma once

#include "move.h"
#include <iostream>
#include <vector>
#include <initializer_list>
using std::vector;
using std::initializer_list;

class MoveList
{
private:
	static constexpr int MAX_KEY_RANGE = 50;
	static constexpr int KEY_STARTING_POINT = 25;

	vector<Move2> moves[8] {};
	short keyMapping[MAX_KEY_RANGE] {0};
	int keyCount = 0;
	size_t size = 0;
	int capacity;

	class const_iterator
	{
	private:
		vector<Move2>::const_iterator it;
		const vector<Move2>* vectors;
		int current;
		const int& capacity;
	public:
		friend class MoveList;
		const_iterator(const vector<Move2>::const_iterator& iter, int curr, const MoveList& list) : it(iter), vectors(list.moves), current(curr), capacity(list.capacity) {}

		using iterator_category = std::forward_iterator_tag;
		using difference_type = std::ptrdiff_t;
		using value_type = Move2;
		using pointer = vector<Move2>::const_iterator;
		using reference = const Move2&;

		reference operator*() const { return *it; }
		pointer operator->() const { return it; }
		const_iterator& operator++()
		{
			it++;
			if (it == vectors[current].end() && current != capacity - 1)
			{
				do {
					current++;
				} while (current < capacity - 1 && vectors[current].size() == 0);
				it = vectors[current].begin();
			}
			return (*this);
		}
		const_iterator operator++(int) { const_iterator tmp = *this; ++(*this); return tmp; }
		friend bool operator== (const const_iterator& a, const const_iterator& b) { return a.current == b.current && a.it == b.it; };
		friend bool operator!= (const const_iterator& a, const const_iterator& b) { return a.current != b.current || a.it != b.it; };
	};

public:
	MoveList(int directions = 8) : capacity(directions) {}
	void setKey(const Square& dir);
	void setKeys(const initializer_list<Square>& dirs) { for (const Square& dir : dirs) setKey(dir); }
	void add(const Square& dir, const Move2& move) { moves[keyMapping[KEY_STARTING_POINT + directionHash(dir)]].push_back(move); size++; }
	void erase(const Square& dir) {
		vector<Move2>& vect = moves[keyMapping[KEY_STARTING_POINT + directionHash(dir)]];
		size -= vect.size();
		vect.clear();
	}
	void clear() { for (int i = 0; i < capacity; i++) moves[i].clear(); size = 0; }
	void reset(int directions = 8);
	const vector<Move2>& getMoves(const Square& dir) { return moves[keyMapping[KEY_STARTING_POINT + directionHash(dir)]]; }
	bool empty() const { return size == 0; }
	const_iterator begin() const
	{
		int i = 0;
		while (i < capacity - 1 && moves[i].size() == 0)
			i++;
		return const_iterator(moves[i].cbegin(), i, *this);
	}
	const_iterator end() const { return const_iterator(moves[capacity - 1].cend(), capacity - 1, *this); }
	static int directionHash(const Square& dir) { return dir.x * 10 + dir.y; }
};

