/* Virtual Machine for Synacor Challenge, take 1 */
#include <iostream>
#include <cstdint>
#include <array>
#include <stack>
#include <vector>
#include <stdexcept>
#include <fstream>
#include <string>
#include <functional>
#include <cstdlib>

// Control what type is used to represent words. Fastest, or exact.
#define FAST_WORD
// Disables some bounds and consistency checks. Should be okay on
// well formed binary images.
#define UNSAFE

#ifdef FAST_WORD
using numtype = std::uint_fast16_t;
#else
using numtype = std::uint16_t;
#endif

#ifdef UNSAFE
#define AT(c, i) c[i]
#else
#define AT(c, i) c.at(i)
#endif

constexpr numtype fix15(numtype a) {
	return a & 0b0111111111111111;
}

constexpr int M{32768}; 

constexpr bool is_number(numtype n) {
	return n < 32768;
}
	
constexpr bool is_register(numtype n) {
	return n > 32767 && n < 32776;
}
	
constexpr int to_register(numtype n) {
	return n - 32768;
}

constexpr bool is_valid(numtype n) {
	return n < 32776;
}

class image {
private:
	using registers = std::array<numtype, 8>;
	using stack = std::stack<numtype>;
	using memory = std::vector<numtype>;
	
	memory mem;
	numtype pc{0};
	registers regs{{0,0,0,0,0,0,0,0}};
	stack s;

	numtype val(numtype);
	numtype load(numtype);
	void store(numtype, numtype);
	void regstore(numtype, numtype);

#define A   pc += 1; numtype a{AT(mem, pc++)}
#define AB  A; numtype b{AT(mem, pc++)}
#define ABC AB; numtype c{AT(mem, pc++)} 
	std::array<std::function<void(void)>, 22>
	ops{{
		[](){ std::exit(0); }, // 0 halt
		[&](){ AB; regstore(a, val(b)); }, // 1 set
		[&](){ A; s.push(val(a)); }, // 2 push
		[&](){ A; if (s.empty()) throw std::runtime_error{"empty stack"};
					 regstore(a, s.top()); s.pop(); }, // 3 pop
		[&](){ ABC; regstore(a, val(b) == val(c)); }, // 4 eq
		[&](){ ABC; regstore(a, val(b) > val(c)); }, // 5 gt
		[&](){ A; pc = val(a); }, // 6 jmp
		[&](){ AB; if (val(a) != 0) pc = val(b); }, // 7 jt
		[&](){ AB; if (val(a) == 0) pc = val(b); }, // 8 jf
		[&](){ ABC; regstore(a, (val(b) + val(c)) % M); }, // 9 add
		[&](){ ABC; regstore(a, (val(b) * val(c)) % M); }, // 10 mult
		[&](){ ABC; regstore(a, val(b) % val(c)); }, // 11 mod
		[&](){ ABC; regstore(a, val(b) & val(c)); }, // 12 and
		[&](){ ABC; regstore(a, val(b) | val(c)); }, // 13 or
		[&](){ AB; regstore(a, fix15(~val(b))); }, // 14 not
		[&](){ AB; regstore(a, load(val(b))); }, // 15 rmem
		[&](){ AB; store(val(a), val(b)); }, // 16 wmem
		[&](){ A; s.push(pc); pc = val(a); }, // 17 call
		[&](){ if (s.empty()) std::exit(0); pc = s.top(); s.pop(); }, // 18 ret
		[&](){ A; std::cout << static_cast<char>(val(a)); }, // 19 out
		[&](){ A; regstore(a, std::cin.get()); }, // 20 in
		[&](){ pc += 1; } // 21 noop
	}};

#undef ABC
#undef AB
#undef A
	
	public:	
		explicit image(std::istream &in);
		void run(void);		
};

image::image(std::istream &in) {
	std::cout << "Reading program..." << std::flush; 
	 do {
	 	 // Read 1k words at a time.
	 	 char raw[sizeof(std::uint16_t) * 1024];
	 	 in.read(raw, sizeof raw);
	 	 if (in.gcount() > 0) {
	 	 	 if (in.gcount() % 2 != 0) {
	 	 	 	 // Couldn't read a full 16 bit word 
	 	 	 	 throw std::runtime_error{"Unable to read full word from input file."};
	 	 	 }
	 	 	 mem.reserve(mem.size() + (in.gcount()/2));
	 	 	 for (int n = 0; n < in.gcount(); n += 2) {
	 	 	 	 numtype word = static_cast<unsigned char>(raw[n])
	 	 	 	 	+ (static_cast<unsigned char>(raw[n+1]) << 8);
	 	 	 	 mem.push_back(word);
	 	 	 }
	 	 }
	 } while (in.good());
	 std::cout << " done. Read " << mem.size() << " words.\n";
}

// Convert a numtype to a number, or return the value
// in a register if numtype refers to one.
numtype image::val(numtype n) {
#ifdef UNSAFE
	if (is_number(n))
		return n;
	else
		return regs[to_register(n)];
#else
	if (is_number(n))
		return n;
	else if (is_register(n))
		return regs[to_register(n)];
	else
		throw std::runtime_error{"Invalid number."};
#endif
}

// Return the value stored in a memory location
numtype image::load(numtype addr) {
	if (mem.size() <= addr)
		return 0;
	else
		return mem[addr];
}

void image::store(numtype addr, numtype v) {
	if (mem.size() <= addr)
		mem.resize(addr + 1);
	mem[addr] = v;
}

// Write val to the register a
void image::regstore(numtype r, numtype val) {
#ifdef UNSAFE
	regs[to_register(r)] = val;
#else
	if (is_register(r))
		regs[to_register(r)] = val;
	else 
		throw std::runtime_error("not a register");
#endif
}

void image::run(void) {
	while (pc < mem.size()) {
		auto insn = mem[pc];
		AT(ops, insn)();
	}
}

int main(int argc, char **argv) {
	if (argc != 2) {
		std::cout << "Usage: " << argv[0] << " IMAGEFILE\n";
		return 1;
	}
	
	std::ifstream input(argv[1], std::ifstream::in | std::ifstream::binary);
	if (!input.is_open()) {
		std::cerr << "Unable to open " << argv[1] << " for reading.\n";
		return 1;
	}
	
	image p{input};
	p.run();
	std::cout << '\n';
	return 0;
}