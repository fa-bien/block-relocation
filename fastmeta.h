#ifndef FASTMETA_H
#define FASTMETA_H

#include "brppolicy.h"

class FastMetaPolicy: public BRPPolicy {
public:
    FastMetaPolicy() {}
    virtual string name() const { return "FastMetaPolicy"; }

    // return number of moves necessary to empty the bay
    virtual shared_ptr<BRPState> solve(const BRPState &s1) const;
    
};

#endif
