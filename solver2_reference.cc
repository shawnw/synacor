#include <iostream>
#include <tuple>
#include <string>
#include <cstdint>
#include <stack>

/* This is a reference version of the teleporter algorithm that's
 * a line by line translation of the original assembly. g++ only,
 * and very slow. 
 *
 * v7 = 2 gives r0 = 13234
 */

using numtype = std::uint_fast16_t;
constexpr numtype M{32768}; 

using stack = std::stack<numtype>;

std::tuple<numtype, numtype> solver_slow(numtype r0, numtype r1, numtype r7) {
	// UGLY CODE ALERT! Using a nice recursive version overflows the stack.
	stack s;
	std::stack<void*> callstack;
	void *lbl;
	
	callstack.push(&&lend);
	
	int n = 0;
	
l178B:
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
	numtype r0, r1, r7;
	
	r7 = std::stoul(argv[1]);
	
	std::tie(r0, r1) = solver_slow(4, 1, r7);
	std::cout << "r0=" << r0 << " r1=" << r1 << " r7=" << r7 << '\n';
	return 0;
}
