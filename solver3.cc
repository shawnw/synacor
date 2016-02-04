#include <iostream>
#include <vector>
#include <utility>
#include <tuple>

// Solver for another puzzle in synacor.
enum class type { NUM, OP };
enum class edgeop { ADD, SUB, MUL };
struct node {
	bool goal;
	type t;
	union {
		edgeop op;
		int val;
	} v;
	node(void) : t(type::NUM), goal(false) { v.val = -1; }
	explicit node(int v_) : t(type::NUM), goal(false) { v.val = v_; }
	explicit node(edgeop o_) : t(type::OP), goal(false) { v.op = o_; }
};

using coords = std::pair<int, int>;
using paths = std::vector<std::vector<coords>>;
using grid = std::vector<std::vector<node>>;

// Paths of this or longer are too long.
constexpr int upper_limit = 14;
constexpr int goal = 30; // Goal weight

void rove(const grid &g, int x, int y, paths &p, int steps, int weight, std::vector<coords> route) {
	if (++steps >= upper_limit) {
		// Too long of a path.
		return;
	}
	if (x == 0 && y == 0 && steps > 1) {
		// Can't go back to the starting point.
		return;
	}
	
	if (g[x][y].t == type::NUM) {
		if (route.empty()) {
			weight = g[x][y].v.val;
		} else {
			int ox, oy;
			std::tie(ox, oy) = route.back();
			switch (g[ox][oy].v.op) {
			case edgeop::ADD:
				weight += g[x][y].v.val;
				break;
			case edgeop::SUB:
				weight -= g[x][y].v.val;
				break;
			case edgeop::MUL:
				weight *= g[x][y].v.val;
				break;
			}
		}
	}
	
	route.emplace_back(x, y);

	if (x == 3 && y == 3) {
		// Going to the ending room is a fail unless weight is right.
		if (weight == goal)
			p.push_back(route);
		return;
	}

	if (x > 0)
		rove(g, x - 1, y, p, steps, weight, route);
	if (x < 3)
		rove(g, x + 1, y, p, steps, weight, route);
	if (y > 0)
		rove(g, x, y - 1, p, steps, weight, route);
	if (y < 3)
		rove(g, x, y + 1, p, steps, weight, route);
}


grid
build_graph(void) {
	grid g(4, std::vector<node>(4));
	g[0][0] = node(22);
	g[0][1] = node(edgeop::SUB);
	g[0][2] = node(9);
	g[0][3] = node(edgeop::MUL);

	g[1][0] = node(edgeop::ADD);
	g[1][1] = node(4);
	g[1][2] = node(edgeop::SUB);
	g[1][3] = node(18);

	g[2][0] = node(4);
	g[2][1] = node(edgeop::MUL);
	g[2][2] = node(11);
	g[2][3] = node(edgeop::MUL);

	g[3][0] = node(edgeop::MUL);
	g[3][1] = node(8);
	g[3][2] = node(edgeop::SUB);
	g[3][3] = node(1);
	g[3][3].goal = true;

	return g;
}
	
int main(void) {
	grid g = build_graph();
	paths p;
	std::vector<coords> r;
	
	rove(g, 0, 0, p, 0, 0, r);
	
	std::cout << "Found " << p.size() << " paths.\n";
	for (auto &path : p) {
		std::cout << "Length: " << path.size() << '\n';
		std::cout << "Route:";
		coords prev = path.front();
		path.erase(path.begin());
		for (auto &c : path) {
			if (prev.first < c.first)
				std::cout << " N";
			else if (prev.first > c.first)
				std::cout << " S";
			else if (prev.second < c.second)
				std::cout << " E";
			else
				std::cout << " W";
			prev = c;
		}
		std::cout << '\n';
	}
	
	return 0;
}