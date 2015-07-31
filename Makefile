all: tagged.accuracy

.DELETE_ON_ERROR:

tagged.accuracy: tagged.xml
	PYTHONIOENCODING=utf8 python ../../PL/tools/corpus2/utils/tagger-eval.py \
	tagged.xml ../../PL/folds/test01.xml | tee $@ | grep AVG

tagged.xml : tagged.idx
	cat $^ | perl scripts/vw2ces.pl -i ../../PL/folds/testana/test01.xml > $@

tagged.idx: trainer data/train01.flat data/test01.flat
	./trainer --train data/train01.flat --test data/test01.flat \
	--passes 1 --history_length 1 --window 1 --model model.weights > tagged.idx

################################################################################

HEADER=src/VowpalTaggit.hpp src/libsearch.h src/Search.hpp src/Examples.hpp src/Features.hpp src/StaticData.hpp
IMPLEM=src/VowpalTaggit.cpp src/Examples.cpp src/Features.cpp src/StaticData.cpp src/Search.cpp

trainer: src/trainer.cpp $(IMPLEM) $(HEADER)
	g++ src/trainer.cpp $(IMPLEM) -std=c++11 -O3 -march=native -fno-align-functions -fno-align-loops -lvw -lpcrecpp -lboost_program_options -o $@

swig: perl/libVowpalTaggit.so

perl/libVowpalTaggit.so: $(IMPLEM) $(HEADER) perl/VowpalTaggit.i
	swig -perl5 -c++ perl/VowpalTaggit.i
	perl -i -pe 's/#include <algorithm>/#undef seed\n#include <algorithm>/' perl/VowpalTaggit_wrap.cxx
	g++ -shared -g -O3 -std=c++11 perl/VowpalTaggit_wrap.cxx $(IMPLEM) -w \
	-I. -Isrc -I/usr/lib/perl/5.18.2/CORE/ -fPIC -lvw -lpcrecpp -lperl -pthread -o perl/libVowpalTaggit.so
	rm -rf perl/VowpalTaggit_wrap.cxx

clean:
	rm -rf trainer tagged.xml tagged.idx tagged.accuracy perl/*.so perl/*.pm

