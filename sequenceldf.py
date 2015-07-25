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

    def makeExamples(self, word, prev1ex, prev2ex):
        examples = []
        chosen = 0
        
        pfeatures = {}
        if prev1ex:
            pfeatures['p'] = [ f for f in  prev1ex[1]['t'] ]
        if prev2ex:
            pfeatures['q'] = [ f for f in  prev2ex[1]['t'] ]

        for i, ldf in enumerate(word):
            label, features = ldf
            fc = features.copy()
            if pfeatures:
                fc.update(pfeatures)
            if label == '1111:0':
                chosen = i
            #print fc;
            ex = self.example(fc, labelType=self.vw.lCostSensitive)
            ex.set_label_string(label)
            examples.append(ex)
        return (chosen, examples)
        
    def _run(self, sentence):   # it's called _run to remind you that you shouldn't call it directly!
        output = []
        for n in range(len(sentence)):
            prev1ex = sentence[n - 1][output[n - 1]] if n > 0 else [];
            prev2ex = sentence[n - 2][output[n - 2]] if n > 1 else [];
            chosen, examples = self.makeExamples(sentence[n], prev1ex, prev2ex)
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
        if c > 10000:
            break
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
vw = pyvw.vw("--search 0 --hash all -b 31 --noconstant --csoaa_ldf mc --quiet --search_task hook -q t:")

# tell VW to construct your search task object
sequenceLabeler = vw.init_search_task(SequenceLabeler)

print "Crunching!"

train = data
step = 100
for i in range(100):
    subtrain = train[step*i:step*i+step]
    sequenceLabeler.learn(subtrain)

    loss = vw.get_sum_loss()
    ex = vw.get_weighted_examples()
    if ex > 0:
        print ("%d : loss: %.4f" % (step*i+step, loss/ex))


#print 'should have printed: [1, 2, 3, 1, 2]'

