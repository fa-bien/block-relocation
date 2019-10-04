#ifndef PILOTMETHOD_H
#define PILOTMETHOD_H

#include "safemoves.h"

class PilotMethod: public SafeMovesPolicy {
public:
    PilotMethod(unsigned int width=10) { width_ = width; }

    // return number of moves necessary to empty the bay
    virtual shared_ptr<BRPState> solve(const BRPState &s1) const;
                                                        
    string name() const { return "pilot method"; }
    
protected:
    unsigned int UB(const BRPState &state,
                    shared_ptr<BRPState> &bestKnown) const;
    virtual vector<pair<int, int> > genSuccMoves(BRPState &state) const;

    unsigned int width_;
};


#endif
