#include <algorithm>

#include "subsequence.h"

extern string condensationProcedure;

ostream& operator<<(ostream &os, const Sequence& s) {
    auto it = s.top();
    while (it != s.bottom()) {
        os << " " << *it++;
    }
    os << " " << *it;
    return os;
}

bool SubsequencePolicy::voluntaryMoves(BRPState &state) const {
    // cout << endl << endl << "Voluntary moves!!!" << endl;
    // cout << state << endl;
    return makeRoomForSubsequence(state);
}

// returns the next decreasing subsequence at stack s
// sequence is from top to bottom so we return reverse iterators
const Sequence SubsequencePolicy::nextDecreasingSequence(BRPState &state,
                                                         unsigned int s) const {
    vector<int>::reverse_iterator seqTop = state.topIt(s);
    vector<int>::reverse_iterator seqBottom = seqTop;
    while ( *(seqBottom + 1) != state.next() &&
            *(seqBottom + 1) <= *seqBottom ) {
        seqBottom++;
    }
    return Sequence(seqTop, seqBottom);
}

// try to relocate the decreasing subsequence blocking state.next()
bool SubsequencePolicy::makeRoomForSubsequence(BRPState &state) const {
    unsigned int s = state.stackForItem(state.next());
    if ( state.top(s) != state.next() ) {
        auto sequence = nextDecreasingSequence(state, s);
        // cout << "Looking for a place to relocate sequence: "
        //      << sequence << endl;
        // we look for a stack where we could fit this subsequence
        int bestNMoves=-1, bestCost=state.H() + 1, bestT=-1;
        for (unsigned int t=0; t < state.W(); t++) {
            if (t != s) {
                // cout << "Trying to relocate " << sequence
                //      << " to stack " << t << endl;
                pair<int, int> tmp = bestSubsequenceFit(state, sequence, t);
                // cout << "\tcost: " << tmp.second << endl;
                if (tmp.second < bestCost) {
                    bestNMoves = tmp.first;
                    bestCost = tmp.second;
                    bestT = t;
                }
            }
        }
        // cout << "Relocating sequence from stack " << s << endl;
        // cout << "\t requires " << bestNMoves << " moves" << endl;
        // now perform these voluntary moves to make room for that sequence
        if (bestNMoves > 0) {
            for (unsigned int i=bestNMoves; i > 0; i--) {
                // cout << "\t relocating " << state.top(bestT) << endl;
                laRelocate(state, state.top(bestT));
            }
        }
    }
    return false;
}

// return the cheapest way to fit decreasing sequence into stack s
// without generating new conflict in s
pair<int, int> SubsequencePolicy::bestSubsequenceFit(BRPState &state, 
                                                     const Sequence &seq,
                                                     unsigned int s ) const {
    // how many free slots to relocate?
    unsigned int nFreeSlots = 0;
    for (unsigned int t=0; t < state.W(); t++) {
        if ( t != s && t != state.stackForItem(state.next()) ) {
            nFreeSlots += state.H() - state.height(t);
        }
    }
    // cout << "\t# free slots: " << nFreeSlots << endl;
    // cout << "\tsequence length: " << seq.size() << endl;
    // cout << "\tsequence top: " << *(seq.top()) << endl;
    // now find where in s we can fit that sequence
    unsigned int position = state.height(s);
    // cout << "\tinitial position: " << position << endl;
    while ( position + seq.size() > state.H() ||
            ( position > 0 &&
              *(seq.bottom()) > *(min_element(state.stacks()[s].begin(),
              // *(seq.top()) > *(min_element(state.stacks()[s].begin(),
                                           state.stacks()[s].begin()
                                           + position) ) &&
              state.height(s) - position < nFreeSlots ) ) {
        position -= 1;
    }
    // cout << "\tposition: " << position << endl;
    unsigned int nToRemove = state.height(s) - position;
    // cout << "\t" << nToRemove << " blocks to relocate" << endl;
    unsigned int cost = 0;
    // for each item to relocate, incur cost
    for (unsigned int i=0; i < nToRemove; i++) {
        unsigned int item = state.itemAt(s, position + i);
        unsigned int presentCost = state.mustBeMoved(item) ? 0 : 1;
        unsigned int futureCost = 1;
        for (unsigned int t=0; t < state.W(); t++) {
            if ( t != s &&
                 state.low(t) >= item &&
                 state.low(t) != state.next() &&
                 state.height(t) < state.H() ) {
                futureCost = 0;
                break;
            }
        }
        cost += presentCost + futureCost;
        // cout << "\tpresent and future cost for item " << item << ": "
        //      << presentCost << " " << futureCost << endl;
        // cout << "\ttotal cost so far: " << cost << endl;
    }
    // it can still happen that we created new conflicts due to not enough
    // free slots in other stacks - count them
    unsigned int localMin;
    if (position == 0) {
        localMin = state.n() + 1;
    } else {
        localMin = *(min_element(state.stacks()[s].begin(),
                                 state.stacks()[s].begin()
                                 + position - 1));
    }
    auto it = seq.top();
    while (it != seq.bottom()) {
        if (*it > localMin) {
            // cout << "*** extra cost for placing " << *it << " on top of "
            //      << localMin << endl;
            cost += 1;
            ++it;
        } else {
            break;
        }
    }
    if (*(seq.bottom()) > localMin) {
        // cout << "*** extra cost for placing " << *it << " on top of "
        //      << localMin << endl;
        cost += 1;
    }
    return make_pair(nToRemove, cost);
}

bool SmartSubsequencePolicy::voluntaryMoves(BRPState &state) const {
    unsigned int s = state.stackForItem(state.next());
    auto sequence = nextDecreasingSequence(state, s);
    if (sequence.size() < 2) {
        if (level_ == 1) {
            return performSafeRelocateTop(state);
        } else {
            return performSafeRelocateTop(state) ||
                performSafe2RelocateTop(state);
        }
    } else {
        return makeRoomForSubsequence(state);
    }
}

shared_ptr<BRPState> SmartSubsequencePolicy::
solve(const BRPState &state) const {
    shared_ptr<BRPState> tmp = SafeMovesPolicy::solve(state);
    if ( condensationProcedure == "tricoire" ) {
        tmp->condenseTricoire();
    } else if ( condensationProcedure == "jin" ) {
        tmp->condenseJin();
    }
    return tmp;
}
