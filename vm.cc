/* Virtual Machine for Synacor Challenge, take 1 */


/* Usage: vm [-s -d -g] IMGFILE
*
* Options: -s IMGFILE is a saved state from a previous session
*          -d Don't dump a saved state on SIGINT.
*          -g Debug mode
*/

/* Normally, when I write an interpeter for a small bytecode language, I
 * use a big switch statement to dispatch, one case per opcode. This time
 * I decided to try something slightly different, though, and use an array
 * of functions, with index corresponding to the opcode. C++11 lambdas make
 * defining this array all happen in one spot, and ends up visually looking
 * a lot like the classic switch. The actual execution of the program turns
 * into two lines of while loop and function call.
 *
 * Possible other directions, for fun and profit: threaded code turning it
 * into an array of functions, and JIT compiling a program to native code.
 */

#include <iostream>
#include <array>
#include <stack>
#include <vector>
#include <deque>
#include <stdexcept>
#include <fstream>
#include <string>
#include <functional>
#include <regex>
#include <cstdint>
#include <cstring>
#include <csignal>

#include <boost/endian/conversion.hpp>

/* Control what type is used to represent words. Fastest for the host system,
   or exactly 16 bits.
*/
#define FAST_WORD
/* Disables some bounds and consistency checks. Should be okay on
   well formed binary images.
*/
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

constexpr numtype M{32768}; 

constexpr numtype fix15(numtype a) {
	return a & 0b0111111111111111;
}

constexpr bool is_number(numtype n) {
	return n < 32768;
}
	
constexpr bool is_register(numtype n) {
	return n > 32767 && n < 32776;
}
	
constexpr int to_register(numtype n) {
	return n - 32768;
}

class image {
private:
	using registers = std::array<numtype, 8>;
	using stack = std::stack<numtype>;
	using memory = std::vector<numtype>;
	using char_pool = std::deque<char>;
	
	memory mem;
	numtype pc{0}, cpc{0};
	registers regs{{0,0,0,0,0,0,0,0}};
	stack s;
	bool debug;
	char_pool input_buffer;

	numtype val(numtype);
	numtype load(numtype);
	void store(numtype, numtype);
	void regstore(numtype, numtype);

	char next_char(void);
	void debugger(const std::string &);

	struct end_of_program{};

#define A   pc += 1; numtype a{AT(mem, pc++)}
#define AB  A; numtype b{AT(mem, pc++)}
#define ABC AB; numtype c{AT(mem, pc++)} 
	std::array<std::function<void(void)>, 22>
	ops{{
		[](){  throw end_of_program(); }, // 0 halt
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
		[&](){ if (s.empty()) throw end_of_program(); pc = s.top(); s.pop(); }, // 18 ret
		[&](){ A; std::cout.put(static_cast<char>(val(a))); }, // 19 out
		[&](){ A; regstore(a, next_char()); }, // 20 in
		[&](){ pc += 1; } // 21 noop
	}};

	friend void sigint_handler(int);

#undef ABC
#undef AB
#undef A
	
	public:	
		explicit image(std::istream &, bool = false, bool = false);
		void run(void);
		void dump(const std::string &s) { dump(s.c_str()); }
		void dump(const char *);
};

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
#ifndef UNSAFE
	if (!is_number(addr))
		throw std::runtime_error("Trying to laod from an invalid address.");
#endif
	if (mem.size() <= addr)
		return 0;
	else
		return mem[addr];
}

// Write a value to a memory location.
void image::store(numtype addr, numtype v) {
#ifndef UNSAFE
	if (!is_number(addr))
		throw std::runtime_error("Trying to store in an invalid address.");
#endif
	if (mem.size() <= addr)
		mem.resize(addr + 1);

	mem[addr] = v;
}

// Write val to the encoded register r
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

// Load image from a file.
image::image(std::istream &in, bool dump, bool dbug) : debug(dbug) {
	std::cout << "Reading program..." << std::flush;

	if (dump) { // Load saved state information at start of image
		in >> pc;
		for (int i = 0; i < 8; i += 1)
			in >> regs[i];
		stack::size_type ssize;
		in >> ssize;
		for (stack::size_type i = 0; i < ssize; i += 1) {
			numtype n;
			in >> n;
			s.push(n);
		}
		while (in.peek() != '~')
			in.get();
		in.get();
	}

	 do {
	 	 // Read 1k words at a time.
	 	 char raw[sizeof(std::uint16_t) * 1024];
	 	 in.read(raw, sizeof raw);
	 	 auto bytes = in.gcount();
	 	 if (bytes > 0) {
	 	 	 if (bytes % 2 != 0) {
	 	 	 	 // Couldn't read a full 16 bit word 
	 	 	 	 throw std::runtime_error{"Unable to read full word from input file."};
	 	 	 }
	 	 	 mem.reserve(mem.size() + (bytes/2));
	 	 	 for (int n = 0; n < bytes; n += 2) {
	 	 	 	 std::uint16_t *word = reinterpret_cast<std::uint16_t*>(raw + n);
	 	 	 	 mem.push_back(boost::endian::little_to_native(*word));
	 	 	 }
	 	 }
	 } while (in.good());
	 std::cout << " done. Read " << mem.size() << " words.\n";
}


// Return the next char to be read from standard input
char image::next_char(void) {
	if (!input_buffer.empty()) {
		char c = input_buffer.front();
		input_buffer.pop_front();
		return c;
	} else {
		while (true) {
			std::string line;
			if (std::getline(std::cin, line) && line.length() > 0) {
				if (debug && line[0] == '~') {
					line.erase(0, 1);
					debugger(line);
				} else {
					std::cerr << "Read: '" << line << "'\n";
					line += '\n';
					input_buffer.assign(std::next(line.begin()), line.end());
					return line[0];
				}
			}	else {
			// Error!
			}
		}
	}
}

void image::debugger(const std::string &cmd) {
	static std::regex dump_re{"dump (.+)"};
	std::smatch fields;

	if (cmd == "quit") {
		std::cout << "DEBUG: Quitting.\n";
		throw end_of_program();
	} else if (std::regex_match(cmd, fields, dump_re)) {
		std::cout << "DEBUG: Dumping state to " << fields[1] << '\n';
		dump(fields[1].str());
	} else {
		std::cout << "DEBUG: Unknown command.\n";
	}
}


// Dump current image to a file
void image::dump(const char *filename) {
	std::ofstream out(filename, std::ofstream::out | std::ofstream::binary);
	if (!out.is_open()) {
		std::cerr << "Unable to open file " << filename << " for writing.\n";
		throw std::runtime_error("Unable to open output file.");
	}

	out << cpc << '\n';

	for (auto r : regs)
		out << r << '\n';

	out << s.size() << '\n';
	stack sr;
	while (!s.empty()) {
		sr.push(s.top());
		s.pop();
	}
	while (!sr.empty()) {
		s.push(sr.top());
		sr.pop();
		out << s.top() << '\n';
	}

	out << "~";

	for (auto w : mem) {
		std::uint16_t word_le =
			boost::endian::native_to_little(static_cast<std::uint16_t>(w));
		out.write(reinterpret_cast<char *>(&word_le), 2);
	}
}

void image::run(void) {
	try {
		while (pc < mem.size()) {
			cpc = pc;
			AT(ops, mem[pc])();
		}
	} catch (end_of_program) {
		return;
	}
}

image *p_capture = nullptr;
void sigint_handler(int) {
	if (p_capture)
		p_capture->dump("save.bin");
	std::exit(0);
}

int main(int argc, char **argv) {
	if (argc < 2) {
		std::cout << "Usage: " << argv[0] << "[-s -d -g] IMAGEFILE\n";
		return 1;
	}
	
	bool debug{false};
	bool saved{false};
	bool nodump{false};

	if (argc > 2) {
		for (int i = 1; i < argc - 1; i += 1) {
			if (std::strcmp(argv[i], "-g") == 0)
				debug = true;
			else if (std::strcmp(argv[i], "-s") == 0)
				saved = true;
			else if (std::strcmp(argv[i], "-d") == 0)
				nodump = true;
			else if (argv[i][0] == '-') {
				std::cerr << "Unknown option '" << argv[i] << "'.\n";
				return 1;
			}
		}
	}
	if (argv[argc - 1][0] == '-') {
		std::cerr << "Missing image file name.\n";
		return 1;
	}

	std::string filename{argv[argc - 1]};
	std::ifstream input(filename, std::ifstream::in | std::ifstream::binary);
	if (!input.is_open()) {
		std::cerr << "Unable to open " << filename << " for reading.\n";
		return 1;
	}

//	if (!nodump)
//		std::signal(SIGINT, sigint_handler);
	
	image p{input, saved, debug};
//	p_capture = &p;
	p.run();
//	p_capture = nullptr;
	std::cout << '\n';
	return 0;
}
