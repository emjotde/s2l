all: tagged.accuracy

.DELETE_ON_ERROR:

tagged.accuracy: tagged.xml
	PYTHONIOENCODING=utf8 python ../../PL/tools/corpus2/utils/tagger-eval.py \
	tagged.xml ../../PL/folds/test01.xml | tee $@ | grep AVG

tagged.xml : tagged.idx
	cat $^ | perl scripts/vw2ces.pl -i ../../PL/folds/testana/test01.xml > $@

tagged.idx: trainer data/train01.flat data/test01.flat
	./trainer --train data/train01.flat --test data/test01.flat --passes 3 > tagged.idx



################################################################################

HEADER=VowpalTaggit.hpp libsearch.h Search.hpp Examples.hpp Features.hpp
IMPLEM=VowpalTaggit.cpp Examples.cpp Features.cpp

trainer: trainer.cpp $(IMPLEM) $(HEADER)
	g++ trainer.cpp $(IMPLEM) -std=c++11 -g -O2 -lvw -lpcrecpp -lboost_program_options -o $@

clean:
	rm -rf trainer tagged.xml tagged.idx tagged.accuracy

swig:
	swig -perl5 -c++ VowpalTaggit.i
	perl -i -pe 's/#include <algorithm>/#undef seed\n#include <algorithm>/' VowpalTaggit_wrap.cxx
	g++ -shared -std=c++11 VowpalTaggit_wrap.cxx VowpalTaggit.cpp libsearch.h -w \
	-I. -I/usr/lib/perl/5.18.2/CORE/ -fPIC -lvw -lpcrecpp -lperl -pthread -o libVowpalTaggit.so
