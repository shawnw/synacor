#include <iostream>
#include <cstdint>
#include <string>
#include <stack>
#include <cstdlib>
#include <utility>
#include <tuple>
#include <map>

using numtype = std::uint_fast16_t;
constexpr numtype M{32768}; 

using regs = std::pair<numtype, numtype>;
using stack = std::stack<numtype>;
using cache = std::map<regs, regs>;

#if 0

// This recursive version runs into stack problems real fast.

regs solver_fast(numtype, numtype, numtype, stack &, cache &);
regs
solver_helper(numtype r0, numtype r1, numtype r7, stack &s, cache &c) {
	// 178B
	if (r0 == 0) {
		r0 = (r1 + 1) % M;
		return std::make_pair(r0, r1);
	}
	// 1793
	if (r1 == 0) {
		r0 -= 1;
		r1 = r7;
		return solver(r0, r1, r7, s, c);
	}
	// 17A0
	s.push(r0);
	r1 -= 1;
	std::tie(r0, r1) = solver(r0, r1, r7, s, c);
	r1 = r0;
	r0 = s.top() - 1;
	s.pop();
	return solver(r0, r1, r7, s, c);
}

regs
solver_fast(numtype r0, numtype r1, numtype r7, stack &s, cache &c) {
	auto cv = c.find(std::make_pair(r0, r1));
	if (cv != c.end()) {
		return cv->second;
	}
	auto res = solver_helper(r0, r1, r7, s, c);
	c.emplace(std::make_pair(r0, r1), res);
	return res;
}
#endif


regs solver(numtype r0, numtype r1, numtype r7, stack &s, cache &c) {
	// UGLY CODE ALERT!
	std::stack<void*> callstack;
	std::stack<regs> regstack;
	void *lbl;
	
	callstack.push(&&lend);
	
	int n = 0;
	
l178B:
	auto cv = c.find(std::make_pair(r0, r1));
	if (cv != c.end()) {
		std::tie(r0, r1) = cv->second;
		lbl = callstack.top();
		callstack.pop();
		goto *lbl;
	}
	
#if 0
	if (s.empty()) 
		std::cout << "a r0=" << r0 << " r1=" << r1 << '\n';
	else
		std::cout << "a r0=" << r0 << " r1=" << r1 << " s=" << s.top() << " sl=" << s.size() << '\n';

	if (++n == 10)
		std::exit(0);
#endif	
	
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
	numtype r0, r1;
	
	if (argc == 2) {
		numtype r7 = std::stoul(argv[1]);
		cache c;
		stack s;
		std::tie(r0, r1) = solver(4, 1, r7, s, c);
		std::cout << "r0=" << r0 << " r1=" << r1 << " r7=" << r7 << '\n';
		return 0;
	}
		
	#pragma omp parallel for
	for (int r7 = 5; r7 < M; r7 += 1) {
		cache c;
		stack s;
		std::tie(r0, r1) = solver(4, 1, r7, s, c);
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