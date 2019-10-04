#ifndef SUBSEQUENCE_H
#define SUBSEQUENCE_H

#include <iostream>

#include "safemoves.h"

using namespace std;

// used to represent a contiguous sequence of items in a stack
class Sequence {
public:
    Sequence(vector<int>::reverse_iterator top,
             vector<int>::reverse_iterator bottom) : top_(top),
                                                     bottom_(bottom) {}

    vector<int>::reverse_iterator top() const { return top_; }
    
    vector<int>::reverse_iterator bottom() const { return bottom_; }

    unsigned int size() const { return 1 + bottom_ - top_; }

protected:
    // top of the sequence
    vector<int>::reverse_iterator top_;
    // bottom of the sequence (points to last element in the sequence)
    vector<int>::reverse_iterator bottom_;
};

ostream& operator<<(ostream &os, const Sequence& s);

class SubsequencePolicy: public SafeMovesPolicy {
public:
    SubsequencePolicy() { }
    virtual string name() const { return "SubsequencePolicy"; }
    
protected:
    virtual bool voluntaryMoves(BRPState &state) const;

    // returns the next decreasing subsequence at stack s
    // sequence is from top to bottom so we return reverse iterators
    const Sequence nextDecreasingSequence(BRPState &state,
                                          unsigned int s) const;
    
    // make room for the first decreasing subsequence that is blocking
    // the next item to be retrieved
    bool makeRoomForSubsequence(BRPState &state) const;

    // return the cheapest way to fit decreasing sequence into stack s
    // without generating new conflict in s
    pair<int, int> bestSubsequenceFit(BRPState &state, 
                                      const Sequence &seq,
                                      unsigned int s ) const;
};

class SmartSubsequencePolicy: public SubsequencePolicy {
public:
    SmartSubsequencePolicy( unsigned int level=1 ) : level_(level) {}
    virtual string name() const { return "SmartSubsequencePolicy(" +
            to_string(level_) + ")"; }
    shared_ptr<BRPState> solve(const BRPState &state) const;
protected:
    unsigned int level_;
    virtual bool voluntaryMoves(BRPState &state) const;
};

#endif
