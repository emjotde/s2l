all: examples test_search_ldf

HEADERS= *.hpp
IMPL = *.cpp

examples : examples.cpp libsearch.h
	g++ examples.cpp -std=c++11 -g -O0 -lvw -o $@

test_search_ldf : test_search_ldf.cc libsearch.h
	g++ test_search_ldf.cc -std=c++11 -lvw -o $@