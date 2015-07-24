all: examples swig

HEADERS= *.hpp
IMPL = *.cpp

swig:
	swig -perl5 -c++ VowpalTaggit.i
	perl -i -pe 's/#include <algorithm>/#undef seed\n#include <algorithm>/' VowpalTaggit_wrap.cxx
	g++ -shared -std=c++11 VowpalTaggit_wrap.cxx VowpalTaggit.cpp libsearch.h \
	-I. -I/usr/lib/perl/5.18.2/CORE/ -fPIC -lvw -lperl -pthread -o libVowpalTaggit.so

examples: examples.cpp VowpalTaggit.cpp libsearch.h
	g++ $^ -std=c++11 -g -O3 -lvw -fPIC -L. -o $@

clean:
	rm -rf libVowpalTaggit.so VowpalTaggit.pm VowpalTaggit_wrap.cxx examples
