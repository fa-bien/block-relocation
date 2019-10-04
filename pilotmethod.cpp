#include <algorithm>

#include "pilotmethod.h"
#include "brppolicy.h"
#include "safemoves.h"
#include "subsequence.h"

extern unique_ptr<BRPPolicy> hubSolver;
extern bool verbose;

shared_ptr<BRPState> PilotMethod::solve(const BRPState &s1) const {
    cout << "Solving with " << name() << endl;
    BRPState state(s1);
    vector<shared_ptr<BRPState> > Q;
    shared_ptr<BRPState> bestKnown = NULL;
    UB(s1, bestKnown);
    Q.push_back(make_shared<BRPState>(state));
    while (true) {
        if ( verbose ) {
            cout << "Size of Q: " << Q.size() << endl;
            cout << "\tBest solution found so far: "
                 << bestKnown->nRelocations()
                 << endl;
        }
        unsigned int bestUB = 1e9;
        vector<shared_ptr<BRPState> > bestCandidates;
            unsigned int nSucc = 0;
        for (auto cs: Q) {
            // cout << "\t\tstate with NR = " << cs->nRelocations()
            //      << ", LB = " << cs->LB3() << endl;

            if (cs->nRelocations() + cs->LB3() >= bestKnown->nRelocations()) {
                // cout << "\t\t --> Skipping!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
                continue;
            }
            
            // step 1: retrieve all items that can be retrieved
            while ( cs->next() <= cs->n() &&
                    cs->next() == cs->top(cs->stackForItem(cs->next()) ) ) {
                cs->pop(cs->stackForItem(cs->next()));
            }
            // are we done? If yes then we won't find better than this solution
            if (cs->empty()) {
                return bestKnown;
            }
            // step 2: try each successor
            for ( auto move: genSuccMoves(*cs) ) {
                nSucc += 1;
                shared_ptr<BRPState> succ = make_shared<BRPState>(*cs);
                succ->relocate(move.first, move.second);

                voluntaryMoves(*succ);
                
                // unsigned int lb = succ->LB();
                unsigned int ub = UB(*succ, bestKnown);
                // we only keep the best
                if ( ub < bestUB ) {
                    bestUB = ub;
                    bestCandidates.clear();
                    bestCandidates.push_back(succ);
                } else if (ub == bestUB) {
                    // we keep all the best though
                    bestCandidates.push_back(succ);
                }
            }
        }

        if (bestCandidates.size() == 0) {
            return bestKnown;
        }
        
        // cout << "\tBest UB at this iteration: " << bestUB << endl;
        // cout << "\tBest solution found so far: " << bestKnown->nRelocations()
        //      << endl;
        // cout << "\tcandidates with same UB: " << bestCandidates.size()
        //      << endl;
        // cout << "\tcandidates in total: " << nSucc
        //      << endl;
        Q.clear();
        // if (bestCandidates.size() > width_) {
        //     random_shuffle(bestCandidates.begin(), bestCandidates.end());
        // }
        for (auto s: bestCandidates) {
            Q.push_back(s);
            // cout << "NR + LB = " << s->nRelocations() + s->LB() << endl;
            // cout << *s << endl;
            if (Q.size() >= width_) {
                break;
            }
        }
    }
    cout << "tralala" << endl;
    return NULL;
}
                                                        
unsigned int PilotMethod::UB(const BRPState &state,
                             shared_ptr<BRPState> &bestKnown) const {
    auto thisResult = hubSolver->solve(state);
    if ( bestKnown == NULL ||
         thisResult->nRelocations() < bestKnown->nRelocations() ) {
        bestKnown = thisResult;
    }
    return thisResult->nRelocations();
}

vector<pair<int, int> > PilotMethod::genSuccMoves(BRPState &state) const {
    vector<pair<int, int> > result;
    // try all valid moves
    for (unsigned int s = 0; s < state.W(); s++) {
        if ( s != state.lastRelocatedTo() && state.height(s) > 0 ) {
            bool toEmpty = false;
            for (unsigned int t = 0; t < state.W(); t++) {
                if (t != s && state.height(t) < state.H()) {
                    if (state.height(t) > 0 || ! toEmpty) {
                        result.push_back(make_pair(s, t));
                        if (state.height(t) == 0) {
                            toEmpty = true;
                        }
                    }
                }
            }
        }
    }
    return result;
}
