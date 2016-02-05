vm: vm.cc
	g++ -O2 -march=native -std=c++11 -W -Wall -o vm vm.cc

solver: solver.cc
	g++ -O2 -march=native -std=c++14 -W -Wall -o solver solver.cc
	
# Add -fopenmp when possible.
solver2: solver2.cc
	g++ -O2 -march=native -std=gnu++11 -W -Wall -o solver2 solver2.cc
	
solver3: solver3.cc
	g++ -O2 -march=native -std=c++11 -W -Wall -o solver3 solver3.cc
