all: tagged.accuracy

.DELETE_ON_ERROR:

tagged.accuracy: tagged.xml
	PYTHONIOENCODING=utf8 python scripts/tagger-eval.py \
	tagged.xml data/test01disamb.xml | tee $@ | grep AVG

tagged.xml : tagged.idx
	cat $^ | perl scripts/vw2ces2.pl -i data/test01.xml --unk-in data/test.unk.predict.txt > $@

tagged.idx: trainer data/train01.flat data/test01.flat
	./trainer --train data/train01.flat_unk --test data/test01.flat_unk5 \
	--window 2 --history-length 3 --classes data/pl.classes \
	--passes 3  --final-model model.weights > tagged.idx

################################################################################

HEADER=src/VowpalTaggit.hpp src/libsearch.h src/Search.hpp src/Examples.hpp src/Features.hpp src/StaticData.hpp
IMPLEM=src/VowpalTaggit.cpp src/Examples.cpp src/Features.cpp src/StaticData.cpp src/Search.cpp

trainer: src/trainer.cpp $(IMPLEM) $(HEADER)
	g++ src/trainer.cpp $(IMPLEM) -std=c++11 -O3 -Ofast -march=native \
	 -fno-align-functions -fno-align-loops -lvw -lpcrecpp \
	 -lboost_program_options -o $@

swig: perl/libVowpalTaggit.so

perl/libVowpalTaggit.so: $(IMPLEM) $(HEADER) perl/VowpalTaggit.i
	swig -perl5 -c++ perl/VowpalTaggit.i
	perl -i -pe 's/#include <algorithm>/#undef seed\n#include <algorithm>/' perl/VowpalTaggit_wrap.cxx
	g++ -shared -g -O3 -std=c++11 perl/VowpalTaggit_wrap.cxx $(IMPLEM) -w \
	-I. -Isrc -I/usr/lib/perl/5.18.2/CORE/ -fPIC -lvw -lpcrecpp -lperl -pthread -o perl/libVowpalTaggit.so
	rm -rf perl/VowpalTaggit_wrap.cxx

clean:
	rm -rf trainer tagged.xml tagged.idx tagged.accuracy perl/*.so perl/*.pm

