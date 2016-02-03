#include <iostream>
#include <cstdint>
#include <string>
#include <stack>
#include <cstdlib>
#include <utility>
#include <tuple>

using numtype = std::uint_fast16_t;
constexpr numtype M{32768}; 

using stack = std::stack<numtype>;

stack s;

std::pair<numtype, numtype> solver(numtype r0, numtype r1, numtype r7) {
	// UGLY CODE ALERT! Using a nice recursive version overflows the stack.
	std::stack<void*> callstack;
	void *lbl;
	
	callstack.push(&&lend);
		
l178B:
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
	callstack.push(&&l179F);
	goto l178B;

l179F:
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
	callstack.push(&&l17B3);
	goto l178B;

l17B3:
	lbl = callstack.top();
	callstack.pop();
	goto *lbl;
	
lend:
	return std::make_pair(r0, r1);
}

int main(int argc, char **argv) {
	numtype r7;
	
	if (argc == 2)
		r7 = std::stoul(argv[1]);
	else
		r7 = 32767;
	
	while (r7 >= 0) {
		numtype r0, r1;
		std::tie(r0, r1) = solver(4, 1, r7);
		if (r0 == 6) {
			std::cout << "r0 = " << r0 << "\nr1 = " << r1 << "\nr7 = " << r7 << '\n';
			return 0;
		}
		r7 -= 1;
	}
}