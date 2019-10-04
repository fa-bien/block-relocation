#ifndef SAFEMOVES_H
#define SAFEMOVES_H

#include "brppolicy.h"

class SafeMovesPolicy: public BRPPolicy {
public:
    SafeMovesPolicy(unsigned int level=2): level_(level) {}
    virtual string name() const { return "SafeMovesPolicy(" +
            to_string(level_) + ")"; }

protected:
    unsigned int level_;

    virtual bool voluntaryMoves(BRPState &state) const;
    
    bool performSafeRelocateTop(BRPState &state) const;
    
    bool performSafe2RelocateTop(BRPState &state) const;
};

#endif
