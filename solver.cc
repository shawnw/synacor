#include <vector>
#include <iostream>
#include <algorithm>
#include <iterator>

constexpr int pow(int x, int y) {
	int p = x;
	while (--y) p *= x;
	return p;
}

bool is_solution(const std::vector<int> &n) {
	int goal = 399;
	return (n[0] + n[1] * pow(n[2],2) + pow(n[3],3) - n[4]) == goal;
}

int main(void) {
	std::vector<int> nums{2,3,5,7,9};
	do {
		if (is_solution(nums)) {
			std::copy(nums.begin(), nums.end(),
				std::ostream_iterator<int>(std::cout, "\n"));
			return 0;
		}
	} while (std::next_permutation(nums.begin(), nums.end()));
	std::cout << "No solution found!\n";
	return 0;
}