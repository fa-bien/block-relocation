#include "fastmeta.h"
#include "safemoves.h"
#include "subsequence.h"

// return number of moves necessary to empty the bay
shared_ptr<BRPState> FastMetaPolicy::solve(const BRPState &s1) const {
    vector<shared_ptr<BRPPolicy> > heuristics( {
            make_shared<SafeMovesPolicy>(1),
                make_shared<SafeMovesPolicy>(2),
                make_shared<SmartSubsequencePolicy>(1),
                make_shared<SmartSubsequencePolicy>(2)
                } );
    shared_ptr<BRPState> best = NULL;
    for (auto thisHeuristic: heuristics) {
        auto thisResult = thisHeuristic->solve(s1);
        if ( best == NULL ||
             thisResult->nRelocations() < best->nRelocations() ) {
            best = thisResult;
        }
    }
    return best;
}
