#pragma once
#include "Vector2D.h"
#include <vector>

using namespace std;

#define NullNode Node({-1,-1})

template<class T1, class T2>
struct duo {
	T1 x;
	T2 y;
	duo(T1 _x, T2 _y) : x{ _x }, y{ _y } {}
};

struct Node {
	Vector2D position;
	vector<Node*> adyacents;
	int terrain;

	Node(Vector2D pos, int ter) : position{ pos }, terrain{ ter } {}
	Node(Vector2D pos) : position{ pos } {}
	Node() = default;

	inline bool operator < (const Node &rhs) {
		return terrain < rhs.terrain;
	}
};

struct CompareNodesByTerrain {
	inline bool operator()(pair<int, Node*> lhs, pair<int, Node*> rhs) {
		return lhs.first > rhs.first;
	}
};