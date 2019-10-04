#ifndef BRANCHANDBOUND_H
#define BRANCHANDBOUND_H

#include <memory>

#include "brpstate.h"
#include "brppolicy.h"

class BranchAndBound: public BRPPolicy {
public:

    BranchAndBound(unsigned int UB,
                   string explorationStrategy,
                   unsigned int timeLimit);
    virtual string name() const { return "BranchAndBound"; }
    
    virtual shared_ptr<BRPState> solve(const BRPState &initialState) const;
    
    static const int bestFirst;
    static const int breadthFirst;
    static const int depthFirst;

protected:
    unsigned int UB_;
    string explorationStrategy_;
    unsigned int timeLimit_;
};

#endif
