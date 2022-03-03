all : fib fibsym

fib : fib.cpp
	c++ --std=c++14 -o fib fib.cpp


fibsym : fibsym.cpp
	c++ --std=c++14 -o fibsym fibsym.cpp -lncurses
