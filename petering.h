#ifndef PETERING_H
#define PETERING_H

#include "brpstate.h"
#include "brppolicy.h"

class LA_N: public BRPPolicy {
public:
    LA_N() { }
    LA_N(unsigned int N) { N_ = N; }

    virtual string name() const { return "LA_N(" + to_string(N_) + ")"; }
    
protected:
    // name of this policy
    string name_ = "LA_N";
    
    unsigned int N_;

    virtual bool voluntaryMoves(BRPState &s) const;
};

#endif
