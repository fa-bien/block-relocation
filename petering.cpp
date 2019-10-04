#include <algorithm>
#include <iostream>

#include "petering.h"

using namespace std;

bool LA_N::voluntaryMoves(BRPState &state) const {
    unsigned Nprime = min(N_, state.nRemaining());
    unsigned int r = 1;
    // here we adjust Nprime
    set<int> snp;
    set<int> complement;
    while(true) {
        // stacks of Nprime lowest items
        snp = state.stacksForLowestItems(Nprime);
        // other stacks that are not full
        complement.clear();
        for (unsigned int s=0; s < state.W(); s++) {
            if (snp.find(s) == snp.end() && state.height(s) < state.H()) {
                complement.insert(s);
            }
        }
        if (complement.empty()) {
            Nprime -= 1;
        } else {
            break;
        }
    }
    // now we select the block to relocate
    vector<int> tops = state.tops(snp);
    std::sort(tops.begin(), tops.end());
    unsigned int n;
    while (true) {
        n = tops[tops.size() - r];
        if (n == state.top(state.stackForItem(state.next()))) {
            break;
        } else {
            unsigned int lenE = 0;
            for (unsigned int s=0; s < state.W(); s++) {
                if ( state.low(s) > n && state.height(s) < state.H() ) {
                    lenE += 1;
                }
            }
            if ( lenE == 0 || n == state.low(state.stackForItem(n)) ) {
                r += 1;
            } else {
                break;
            }
        }
    }
    laRelocate(state, n);
    return true;
}
