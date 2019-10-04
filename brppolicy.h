#ifndef BRPPOLICY_H
#define BRPPOLICY_H

#include <string>
#include <iostream>

#include "brpstate.h"

using namespace std;

class BRPPolicy {
public:
    BRPPolicy() { }

    // return number of moves necessary to empty the bay
    virtual shared_ptr<BRPState> solve(const BRPState &s1) const;

    virtual string name() const { return "base policy"; }
    
protected:
    // relocate container n as in LA heuristics
    void laRelocate(BRPState &s, unsigned int n) const;

    // returns the best destination stack for LA-relocating n
    unsigned int bestDestForLaRelocate(BRPState &state,
                                       unsigned int n) const;
    
    // perform a forced move
    // meant to be overloaded in derived classes
    virtual void forcedMove(BRPState &s) const;

    // perform voluntary moves
    // returns true if a voluntary move has been performed, false otherwise
    // This method is meant to be overloaded in derived classes
    virtual bool voluntaryMoves(BRPState &s) const { return false; }

    // retrieve items that can be retrieved
    void autoRetrieve(BRPState &state) const;

};

#endif
