import sys
import pyvw
from random import shuffle

class SequenceLabeler(pyvw.SearchTask):
    def __init__(self, vw, sch, num_actions):
        # you must must must initialize the parent class
        # this will automatically store self.sch <- sch, self.vw <- vw
        pyvw.SearchTask.__init__(self, vw, sch, num_actions)
        
        # set whatever options you want
        sch.set_options( sch.AUTO_HAMMING_LOSS | sch.IS_LDF )    

    def makeExamples(self, word, prev, prev2):
        examples = []
        chosen = 0
        
        pfeatures = {}
        if prev:
            for ns in prev[1]:
                if ns in ['t', 's', 'c']:
                    pfeatures['p' + ns] = [ 'p^' + f for f in  prev[1][ns]]
        if prev2:
            for ns in prev2[1]:
                if ns in ['t', 's', 'c']:
                    pfeatures['q' + ns] = [ 'q^' + f for f in  prev2[1][ns]]
        
        for i, ldf in enumerate(word):
            label, features = ldf
            fc = features.copy()
            if pfeatures:
                fc.update(pfeatures)
            else:
                fc['ps'] = ['p^<s>']
            fc['m'] = []
            if prev:
                fc['m'] += [ 'mt^' + f for f in set(fc['t']) & set(prev[1]['t'])]
            if prev2:
                fc['m'] += [ 'mt2^' + f for f in set(fc['t']) & set(prev2[1]['t'])]
            if prev and prev2:
                fc['m'] += [ 'mt3^' + f for f in set(prev[1]['t']) & set(prev2[1]['t'])]
            
            if label == 'morf:0':
                chosen = i
            ex = self.example(fc, labelType=self.vw.lCostSensitive)
            ex.set_label_string(label)
            examples.append(ex)
        return (chosen, examples)
        
    def _run(self, sentence):   # it's called _run to remind you that you shouldn't call it directly!
        output = []
        for n in range(len(sentence)):
            prevEx2 = sentence[n - 2][output[n - 2]] if n > 1 else [] 
            prevEx1 = sentence[n - 1][output[n - 1]] if n > 0 else [] 
            chosen, examples = self.makeExamples(sentence[n], prevEx1, prevEx2)
            pred = self.sch.predict(examples=examples, my_tag=n+1, oracle=chosen, condition=[(n,'p'), (n-1, 'q')])
            output.append(pred)
        return output

def parseExample(tokens):
    example = {}
    label = tokens[0]
    for token in tokens[1:]:
        if token[:1] == "|":
            namespace = token[1:]
            example[namespace] = []
        else:
            example[namespace].append(token)
    return label, example

data = []
sentence = []
ldf_example = []
c = 0
for line in sys.stdin:
    if '#EOS' in line:
        c += 1
        if c % 1000 == 0:
            print c
        data.append(sentence)
        sentence = []
        continue
    
    tokens = line.split()
    if len(tokens) > 0:
        label, ex = parseExample(tokens);
        ldf_example.append((label, ex))
    else:
        if ldf_example:
            shared = ldf_example[0]
            ldf = ldf_example[1:]
            for l in ldf:
                for k in shared[1]:
                    l[1][k] = shared[1][k] 
            sentence.append(ldf)
            ldf_example = []


# initialize VW as usual, but use 'hook' as the search_task
#vw = pyvw.vw("--search 0 --hash all -b 31 --csoaa_ldf mc --quiet --search_task hook -q t: -q m: --ngram t2 --ngram m2 --ngram g2 --ngram c2")
vw = pyvw.vw("--search 0 --hash all -b 31 --csoaa_ldf mc --quiet --search_task hook -q ::")

# tell VW to construct your search task object
sequenceLabeler = vw.init_search_task(SequenceLabeler)


train = data[:77072]
test  = data[77072:]


def prepare(test):
  for s in range(len(test)):
    sentence = test[s]
    oracle = []
    for w in range(len(sentence)):
      word = sentence[w]
      isIgn = False
      p = 0
      for i,(l,f) in enumerate(word):
        if l == 'morf:0':
          p = i
        if 'tag^ign' in f['t']:
          p = i
          isIgn = True
          break
      if isIgn:
        ignEx = word[p]
        sentence[w] = [ignEx]
        p = - 1
      oracle.append(p)
    test[s] = ( sentence, oracle )

prepare(test)

step = 500
strides = len(data)/step
for i in range(strides):
    
    subtrain = train[step*i:step*i+step]
    sequenceLabeler.learn(subtrain)

    loss = vw.get_sum_loss()
    ex = vw.get_weighted_examples()
    if ex > 0:
        print ("%d : loss: %.4f" % (step*i+step, loss/ex))

    correct = 0
    total = 0
    correctK = 0
    totalK = 0
    # now see the predictions on a test sentence
    print >>sys.stderr, 'predicting!'
    for i, sentence in enumerate(test):
        
        s, o = sentence
        p = sequenceLabeler.predict(s)
        print p
        #print "Oracle:  ", oracle
        #print "OracleK:  ", oracleK
        #
        #print "Predict: ", predicted
        
        correctK += len([ j  for i, j in zip(p, o) if i == j ])
        totalK += len([ j for j in o if j >= 0 ])
        
    print "AccuracyK: ", correctK/float(totalK)

print >>sys.stderr, 'predicting!'
for i, sentence in enumerate(test):
    s, o = sentence
    p = sequenceLabeler.predict(s)
    correctK += len([ j  for i, j in zip(p, o) if i == j ])
    totalK += len([ j for j in o if j >= 0 ])
print "AccuracyK: ", correctK/float(totalK)


#print 'should have printed: [1, 2, 3, 1, 2]'

