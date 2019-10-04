#include <iostream>

#include "safemoves.h"

using namespace std;

bool SafeMovesPolicy::voluntaryMoves(BRPState &state) const {
    if (level_ == 1) {
        return performSafeRelocateTop(state);
    } else if (level_ == 2) {
        return performSafeRelocateTop(state) || performSafe2RelocateTop(state);
    } else {
        cerr << "Illegal level in SafeMoves: " << level_ << endl;
        exit(22);
    }
}

bool SafeMovesPolicy::performSafeRelocateTop(BRPState &state) const {
    tuple<int, int, int> bestSafeRelocate = state.bestSafeRelocate();
    if ( get<0>(bestSafeRelocate) >= 0 ) {
        // cout << "Safe 1-relocate: " << get<0>(bestSafeRelocate)
        //      << " --> " << get<1>(bestSafeRelocate) << endl;
        state.relocate( get<0>(bestSafeRelocate), get<1>(bestSafeRelocate) );
        return true;
    } else {
        return false;
    }
}

bool SafeMovesPolicy::performSafe2RelocateTop(BRPState &state) const {
    tuple<int, int, int> bestSafe2Relocate = state.bestSafe2Relocate();
    if ( get<0>(bestSafe2Relocate) >= 0 ) {
        // cout << "Safe 2-relocate: " << get<0>(bestSafe2Relocate)
        //      << " --> " << get<1>(bestSafe2Relocate) << endl;
        state.relocate( get<0>(bestSafe2Relocate), get<1>(bestSafe2Relocate) );
        return true;
    } else {
        return false;
    }
}
