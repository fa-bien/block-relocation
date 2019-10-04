#include "fastmeta.h"
#include "rakesearch.h"
#include "subsequence.h"


// return number of moves necessary to empty the bay
shared_ptr<BRPState> RakeSearch::solve(const BRPState &s1) const {
    // cout << "Solving with " << name() << endl;
    BRPState state(s1);
    deque<shared_ptr<BRPState> > Q;
    Q.push_back(make_shared<BRPState>(state));
    while (Q.size() < width_) {
        // cout << "Q.size() = " << Q.size() << endl;
        // we want to process the whole content of Q at once to generate the
        // next tree level
        unsigned int nRemaining = Q.size();
        while (nRemaining > 0) {
            // retrieve next (current) state to process
            auto cs = Q.front();
            Q.pop_front();
            --nRemaining;
            // step 1: retrieve all items that can be retrieved
            while ( cs->next() <= cs->n() &&
                    cs->next() == cs->top(cs->stackForItem(cs->next()) ) ) {
                cs->pop(cs->stackForItem(cs->next()));
            }
            // are we done? If yes then we won't find better than this solution
            if (cs->empty()) {
                return cs;
            } else { // general case: generate successors
                for (auto move: genSuccMoves(*cs)) {
                    unsigned int item = cs->top(move.first);
                    shared_ptr<BRPState> succ = make_shared<BRPState>(*cs);
                    // cout << "Trying to relocate from " << move.first
                    //      << " to " << move.second << endl;
                    succ->relocate(move.first, move.second);
                    updateQueue(Q, nRemaining, succ);
                    // cout << "\t --> after update, Q.size() - nRemaining = "
                    //      << Q.size() - nRemaining << endl;
                }
            }
        }
    }

    // cout << "Now trying to finish all " << Q.size() << " partial solutions" << endl;
    
    // at this point, we have generated enough partial solutions
    // now we finish them with heuristics
    FastMetaPolicy fm;
    shared_ptr<BRPState> best = NULL;
    // cout << "Done with the tree, applying " << heuristics.size()
    //      << " heuristics to " << Q.size() << " partial solutions" << endl;
    for (auto thisState: Q) {
        auto thisResult = fm.solve(*thisState);
        if ( best == NULL ||
             thisResult->nRelocations() < best->nRelocations() ) {
            // cout << "new best: " << thisResult << endl;
            // cout << "\t(using " << thisHeuristic->name() << ")" << endl;
            best = thisResult;
        }
    }
    return best;
}

void RakeSearch::updateQueue(deque<shared_ptr<BRPState> > &Q,
                             unsigned int startPosition,
                             shared_ptr<BRPState> state) const {
    // cout << "updating queue with NR + LB = "
    //      << state->nRelocations() + state->LB() << endl;
    unsigned int i = startPosition;
    while (i < Q.size()) {
        if ( Q[i]->dominates(*state) ) {
            // cout << "\tdominated!" << endl;
            return;
        } else if ( state->dominates(*Q[i]) ) {
            // cout << "\tnew solution dominates old one!" << endl;
            if (i == Q.size() -1) {
                Q[i] = state;
                // cout << "\tdominating last one!" << endl;
                return;
            } else {
                // cout << "\tdominating in the middle!" << endl;
                Q[i] = Q.back();
                Q.pop_back();
            }
        } else {
            ++i;
        }
    }
    // cout << "\tadding solution at the end!" << endl;
    Q.push_back(state);
}

vector<pair<int, int> > RakeSearch::genSuccMoves(BRPState &state) const {
    // we store here all generated relocates, in the order in which they
    // were generated
    vector<pair<int, int> > result;
    // keep track of those already generated
    set<pair<int, int> > alreadyGenerated;
    // add all safe 1-relocates
    auto safe1Relocates = state.safeRelocates();
    for (auto t: *safe1Relocates) {
        // cout << "\tAdding safe 1-relocate from " << get<0>(t)
        //      << " to " << get<1>(t) << endl;
        result.push_back( make_pair(get<0>(t), get<1>(t)) );
        alreadyGenerated.insert( make_pair(get<0>(t), get<1>(t)) );
    }
    // add all safe 2-relocates
    auto safe2Relocates = state.safe2Relocates();
    for (auto t: *safe2Relocates) {
        // cout << "\tAdding safe 2-relocate from " << get<0>(t)
        //      << " to " << get<1>(t) << endl;
        auto thisPair = make_pair(get<0>(t), get<1>(t));
        if ( alreadyGenerated.find(thisPair) == alreadyGenerated.end() ) {
            result.push_back( thisPair );
            alreadyGenerated.insert( thisPair );
        }
    }
    // if we still haven't generated enough, we add forced moves as well
    // cout << "\tresult.size() = " << result.size() << endl;
    // cout << "\tstate.W() = " << state.W() << endl;
    vector<unsigned int> allFrom;
    int nMax = min(state.next() + 4, state.n());
    for (int i=state.next(); i <= nMax; i++) {
        allFrom.push_back(state.stackForItem(i));
    }
    // if (result.size() < state.W()) {
    //     // int ns = state.stackForItem(state.next() + 1);
    //     // if (ns != state.lastRelocatedTo()) {
    //     //     allFrom.push_back(ns);
    //     // }
    //     for (unsigned int ns = 0; ns < state.W(); ns++) {
    //         if ( ns != state.stackForItem(state.next())
    //              && ns != state.lastRelocatedTo()
    //              && state.height(ns) > 0
    //              ) {
    //             allFrom.push_back(ns);
    //         }
    //     }        
    // }
    for (auto s : allFrom) {
        for (unsigned int t = 0; t < state.W(); t++) {
            if (t != s && state.height(t) < state.H()) {
                auto thisPair = make_pair(s, t);
                // cout << "\t\tAdding forced move from " << s
                //      << " to " << t << endl;
                if ( alreadyGenerated.find(thisPair) ==
                     alreadyGenerated.end() ) {
                    result.push_back( thisPair );
                    alreadyGenerated.insert( thisPair );
                    // cout << "\tForced move from " << s << " to " << t << endl;
                }
            }
        }
    }
    // return all generated relocates
    return result;
}
