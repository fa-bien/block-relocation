#ifndef RAKESEARCH_H
#define RAKESEARCH_H

#include <deque>

#include "safemoves.h"

class RakeSearch: public SafeMovesPolicy {
public:
    RakeSearch(unsigned int width=50) { width_ = width; }

    // return number of moves necessary to empty the bay
    virtual shared_ptr<BRPState> solve(const BRPState &s1) const;
                                                        
    string name() const { return "rake search"; }
    
protected:
    unsigned int width_;

    void updateQueue(deque<shared_ptr<BRPState> > &Q,
                     unsigned int startPosition,
                     shared_ptr<BRPState> state) const;

    virtual vector<pair<int, int> > genSuccMoves(BRPState &state) const;    
};

#endif
