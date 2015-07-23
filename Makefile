all: search_test test_search test_search_ldf

HEADERS= *.hpp
IMPL = *.cpp

search_test : $(IMPL) $(HEADERS)
	g++ -std=c++11 $(IMPL) -lvw -o $@

test_search : test_search.cc libsearch.h
	g++ test_search.cc -std=c++11 -lvw -o $@

test_search_ldf : test_search_ldf.cc libsearch.h
	g++ test_search_ldf.cc -std=c++11 -lvw -o $@