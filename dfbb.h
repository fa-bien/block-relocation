#ifndef DFBB_H
#define DFBB_H

// depth-first branch-and-bound

#include <memory>

#include "brpstate.h"
#include "brppolicy.h"

class DFBB: public BRPPolicy {
public:

    DFBB(){}
    DFBB(unsigned int UB, unsigned int timeLimit);
    
    virtual string name() const { return "DFBB"; }
    
    virtual shared_ptr<BRPState> solve(const BRPState &initialState) const;

    // returns false if time limit reached, true otherwise
    // side effect: bestFound and bestObj are updated if a new better solution
    // is found
    virtual bool solveSub(BRPState &currentState,
                          unsigned int lastRelocatedTo,
                          shared_ptr<BRPState> &bestFound,
                          unsigned int &bestObj,
                          const clock_t &startTicks) const;
    
protected:
    unsigned int UB_;
    unsigned int timeLimit_;
};

class DFBBLoop: public DFBB {
public:
    DFBBLoop(unsigned int UB, unsigned int timeLimit);
    virtual string name() const { return "DFBB (loop)"; }
    virtual shared_ptr<BRPState> solve(const BRPState &initialState) const;
    
// protected:
//     unsigned int UB_;
//     unsigned int timeLimit_;
};

#endif
