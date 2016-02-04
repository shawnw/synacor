#include <iostream>
#include <cstdint>
#include <string>
#include <stack>
#include <cstdlib>
#include <utility>
#include <tuple>
#include <map>

/* Code is G++ specific. Also works better with OpenMP turned on. */

using numtype = std::uint_fast16_t;
constexpr numtype M{32768}; 

using regs = std::pair<numtype, numtype>;
using stack = std::stack<numtype>;
using cache = std::map<regs, regs>;

// This is the reference version adapted to cache calculations, which speeds
// it up tremendously.
regs solver(numtype r0, numtype r1, numtype r7) {
	// UGLY CODE ALERT!
	stack s;
	cache c;
	std::stack<void*> callstack;
	std::stack<regs> regstack;
	void *lbl;
	callstack.push(&&lend);
	
l178B:
	auto cv = c.find(std::make_pair(r0, r1));
	if (cv != c.end()) {
		std::tie(r0, r1) = cv->second;
		lbl = callstack.top();
		callstack.pop();
		goto *lbl;
	}
	
	if (r0 != 0)
		goto l1793;
	r0 = (r1 + 1) % M;
	lbl = callstack.top();
	callstack.pop();
	goto *lbl;
	
l1793:
	if (r1 != 0)
		goto l17A0;
	r0 -= 1;
	r1 = r7;
	regstack.push(std::make_pair(r0, r1));
	callstack.push(&&l179F);
	goto l178B;

l179F:
	if (c.count(regstack.top()) == 0)
		c.emplace(regstack.top(), std::make_pair(r0, r1));
	regstack.pop();
	lbl = callstack.top();
	callstack.pop();
	goto *lbl;

l17A0:
	s.push(r0);
	r1 -= 1;
	callstack.push(&&l17A8);
	goto l178B;

l17A8:
	r1 = r0;
	r0 = s.top(); 
	s.pop();
	r0 -= 1;
	regstack.push(std::make_pair(r0, r1));
	callstack.push(&&l17B3);
	goto l178B;

l17B3:
	if (c.count(regstack.top()) == 0)
		c.emplace(regstack.top(), std::make_pair(r0, r1));
	regstack.pop();
	lbl = callstack.top();
	callstack.pop();
	goto *lbl;
	
lend:
	return std::make_pair(r0, r1);
}


int main(int argc, char **argv) {

	std::cout.setf(std::ios::showbase);
	std::cout << std::hex;
	
	if (argc == 2) {
		numtype r0, r1;
		numtype r7 = std::stoul(argv[1]);
		cache c;
		stack s;
		std::tie(r0, r1) = solver(4, 1, r7);
		std::cout << "r0=" << r0 << "\nr1=" << r1 << "\nr7=" << r7 << '\n';
		return 0;
	}
		
	#pragma omp parallel for
	for (int r7 = 5; r7 < M; r7 += 1) {
		numtype r0, r1;
		cache c;
		stack s;
		std::tie(r0, r1) = solver(4, 1, r7);
		if (r0 == 6) {
			std::cout << "\nr0 = " << r0 << "\nr1 = " << r1 << "\nr7 = " << r7 << '\n';
			std::exit(0);
		} else {
			if ((r7 % 250) == 0) 
				std::cout << '.' << std::flush;
		}
	}
	return 0;
}