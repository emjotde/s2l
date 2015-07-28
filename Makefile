all: tagged.accuracy

.DELETE_ON_ERROR:

tagged.accuracy: tagged.xml
	PYTHONIOENCODING=utf8 python ../../PL/tools/corpus2/utils/tagger-eval.py \
	tagged.xml ../../PL/folds/test01.xml | tee $@ | grep AVG

tagged.xml : tagged.idx
	cat $^ | perl scripts/vw2ces.pl -i ../../PL/folds/testana/test01.xml > $@

#tagged.idx: predictor model.weights data/test01.flat
#	cat data/test01.flat | ./predictor -t -i model.weights > $@
#
#model.weights: trainer data/train01.flat
#	cat data/train01.flat | ./trainer -f $@

tagged.idx: trainer data/train01.flat
	cat data/train01.flat | ./trainer > tagged.idx

HEADER=VowpalTaggit.hpp libsearch.h Search.hpp Examples.hpp 
IMPLEM=VowpalTaggit.cpp Examples.cpp

trainer: trainer.cpp $(IMPLEM) $(HEADER)
	g++ trainer.cpp $(IMPLEM) -std=c++11 -g -O2 -lvw -lpcrecpp -lboost_program_options -o $@

predictor: predictor.cpp  $(IMPLEM) $(HEADER)
	g++ predictor.cpp $(IMPLEM) -std=c++11 -g -O2 -lvw -lpcrecpp -lboost_program_options -o $@

clean:
	rm -rf trainer predictor model.weights tagged.xml


#swig:
#	swig -perl5 -c++ VowpalTaggit.i
#	perl -i -pe 's/#include <algorithm>/#undef seed\n#include <algorithm>/' VowpalTaggit_wrap.cxx
#	g++ -shared -std=c++11 VowpalTaggit_wrap.cxx VowpalTaggit.cpp libsearch.h -w \
#	-I. -I/usr/lib/perl/5.18.2/CORE/ -fPIC -lvw -lperl -pthread -o libVowpalTaggit.so
