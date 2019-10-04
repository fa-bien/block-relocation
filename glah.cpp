#include <algorithm>
#include <cmath>

#include "glah.h"

extern bool verbose;

// used as UB
shared_ptr<BRPState> JZW::solve(const BRPState &initialState) const {
    BRPState state(initialState);
    autoRetrieve(state);
    while (! state.empty()) {
        int cStar = state.next();
        unsigned int sStar = state.stackForItem(cStar);
        while (state.top(sStar) != cStar) {
            int c = state.top(sStar);

        // cout << state << endl;
        // cout << "s* = " << sStar << endl;
        // cout << "c* = " << cStar << endl;
        // cout << "c = " << c << endl;
            
            // look for stacks that can support c
            int sPrime = -1;
            for (int s=0; s < state.W(); s++) {
                if ( s != sStar &&
                     state.height(s) < state.H() &&
                     state.low(s) >= c ) {
                    if ( sPrime == -1 ||
                         state.low(s) < state.low(sPrime) ) {
                        sPrime = s;
                    }
                }
            }
            // case where S1 is not empty
            if (sPrime > -1) {
                // cout << "S1 not empty!" << endl;
                gapUtilize(state, sStar, sPrime);
                // cout << "\trelocating from " << sStar
                //      << " to " << sPrime << endl;
                state.relocate(sStar, sPrime);
            } else {
                // we look for an assistant stack which we call 'sa'
                int sa = -1;
                for (int s=0; s < state.W(); s++) {
                    if ( s != sStar &&
                         state.height(s) >= 1 &&
                         state.top(s) <= state.f(s) &&
                         c <= state.f(s) ) {
                        for (int t=0; t < state.W(); t++) {
                            if ( t != sStar &&
                                 t != s &&
                                 state.height(t) < state.H() &&
                                 state.low(t) >= state.top(s) ) {
                                if ( sPrime == -1 ||
                                     state.top(s) > state.top(sPrime) ||
                                     ( state.top(s) >= state.top(sPrime) &&
                                       state.low(t) < state.low(sa) ) ) {
                                    sPrime = s;
                                    sa = t;
                                }
                            }
                        }
                    }
                } // case where S2 is not empty
                if (sPrime > -1) {
                    // cout << "S2 not empty!" << endl;
                    int cPrime = state.top(sPrime);
                    gapUtilize(state, sPrime, sa);
                    // cout << "\trelocating from " << sPrime
                    //      << " to " << sa << endl;
                    state.relocate(sPrime, sa);
                    gapUtilize(state, sStar, sPrime);
                    state.relocate(sStar, sPrime);
                    // cout << "\trelocating from " << sStar
                    //      << " to " << sPrime << endl;
                } else { // general case: we have to consider FT-BB relocations
                    // cout << "S1 and S2 both empty!" << endl;
                    int smallestAbove = state.smallestAbove(cStar);
                    vector<pair<int, int> > minAndStack;
                    for (int s=0; s < state.W(); s++) {
                        if ( s != sStar &&
                             state.height(s) < state.H() ) {
                            minAndStack.push_back(make_pair(state.low(s), s));
                        }
                    }
                    sort(minAndStack.begin(), minAndStack.end());
                    // we want the stack with the largest min
                    sPrime = minAndStack.rbegin()->second;
                    // special case as in Jin et al.
                    if ( state.height(sPrime) == state.H() - 1 &&
                         c != smallestAbove ) {
                        sPrime = (minAndStack.rbegin() + 1)->second;
                    }
                    state.relocate(sStar, sPrime);
                    // cout << "\trelocating from " << sStar
                    //      << " to " << sPrime << endl;
                }
            }
        }
        autoRetrieve(state);
    }
    return make_shared<BRPState>(state);
}

// returns the next relocation
pair<int, int> JZW::solveOnlyOne(const BRPState &initialState) const {
    BRPState state(initialState);
    autoRetrieve(state);
    int cStar = state.next();
    unsigned int sStar = state.stackForItem(cStar);
    int c = state.top(sStar);
    // look for stacks that can support c
    int sPrime = -1;
    for (int s=0; s < state.W(); s++) {
        if ( s != sStar &&
             state.height(s) < state.H() &&
             state.low(s) >= c ) {
            if ( sPrime == -1 ||
                 state.low(s) < state.low(sPrime) ) {
                sPrime = s;
            }
        }
    }
    // case where S1 is not empty
    if (sPrime > -1) {
        auto nextReloc = gapUtilizeOnlyOne(state, sStar, sPrime);
        if ( nextReloc.first == -1 ) {
            // case where gapUtilize would not perform any relocation
            return make_pair(sStar, sPrime);
        } else {
            return nextReloc;
        }
    } else {
        // we look for an assistant stack which we call 'sa'
        int sa = -1;
        for (int s=0; s < state.W(); s++) {
            if ( s != sStar &&
                 state.height(s) >= 1 &&
                 state.top(s) <= state.f(s) &&
                 c <= state.f(s) ) {
                for (int t=0; t < state.W(); t++) {
                    if ( t != sStar &&
                         t != s &&
                         state.height(t) < state.H() &&
                         state.low(t) >= state.top(s) ) {
                        if ( sPrime == -1 ||
                             state.top(s) > state.top(sPrime) ||
                             ( state.top(s) >= state.top(sPrime) &&
                               state.low(t) < state.low(sa) ) ) {
                            sPrime = s;
                            sa = t;
                        }
                    }
                }
            }
        } // case where S2 is not empty
        if (sPrime > -1) {
            int cPrime = state.top(sPrime);
            auto nextReloc = gapUtilizeOnlyOne(state, sPrime, sa);
            if ( nextReloc.first == -1 ) {
                return make_pair(sPrime, sa);
            } else {
                return nextReloc;
            }
        } else { // general case: we have to consider FT-BB relocations
            // cout << "S1 and S2 both empty!" << endl;
            int smallestAbove = state.smallestAbove(cStar);
            vector<pair<int, int> > minAndStack;
            for (int s=0; s < state.W(); s++) {
                if ( s != sStar &&
                     state.height(s) < state.H() ) {
                    minAndStack.push_back(make_pair(state.low(s), s));
                }
            }
            sort(minAndStack.begin(), minAndStack.end());
            // we want the stack with the largest min
            sPrime = minAndStack.rbegin()->second;
            // special case as in Jin et al.
            if ( state.height(sPrime) == state.H() - 1 &&
                 c != smallestAbove ) {
                sPrime = (minAndStack.rbegin() + 1)->second;
            }
            return make_pair(sStar, sPrime);
        }
    }
}

// returns the next relocation that would be performed by gapUtilize,
// or <-1, -1> if it would not perform any relocation
pair<int, int> JZW::gapUtilizeOnlyOne(BRPState &state,
                                         unsigned int s1,
                                         unsigned int s2) const {
    while ( state.height(s2) <= state.H() - 2 ) {
        int s3 = -1;
        for (int s=0; s < state.W(); s++) {
            if ( s != s1 && s != s2 && state.height(s) > 1 &&
                 state.top(s) > state.f(s) &&
                 state.top(s1) <= state.top(s) &&
                 state.top(s) <= state.low(s2) ) {
                if ( s3 == -1 || state.top(s) >= state.top(s3) ) {
                    s3 = s;
                }
            }
        }
        if ( s3 == -1 ) {
            return make_pair(-1, -1);
        } else {
            return make_pair(s3, s2);
        }
    }
    return make_pair(-1, -1);
}

GLAH::GLAH(unsigned int level) {
    D_ = level;
    // following the Jin et al. article
    nFTBG_ = 5;
    nNFBG_ = 5;
    nFTBB_ = 3;
    nNFBB_ = 3;
    nGG_ = 1;
    nGB_ = 1;
}

// as in the article
void JZW::gapUtilize(BRPState &state,
                       unsigned int s1, unsigned int s2) const {
    // cout << "\tIn Gap-utilize with s1 = " << s1 << " and s2 = " << s2 << endl;
    while ( state.height(s2) <= state.H() - 2 ) {
        int s3 = -1;
        for (int s=0; s < state.W(); s++) {
            if ( s != s1 && s != s2 && state.height(s) > 1 &&
                 state.top(s) > state.f(s) &&
                 state.top(s1) <= state.top(s) &&
                 state.top(s) <= state.low(s2) ) {
                if ( s3 == -1 || state.top(s) >= state.top(s3) ) {
                    s3 = s;
                }
            }
        }
        if ( s3 == -1 ) {
            return;//break;
        } else {
            state.relocate(s3, s2);
            // cout << "\t\trelocating from " << s3 << " to " << s2 << endl;
        }
    }
}

// wrapped by solve()
shared_ptr<BRPState> GLAH::greedy(const BRPState &initialState) const {
    
    auto solBest = ubSolver_.solve(initialState);

    // cout << "Initialised solBest, nRelocations =  "
    //      << solBest->nRelocations() << endl;
    
    // incumbent
    BRPState Lcurr(initialState);
    autoRetrieve(Lcurr);
    while (! Lcurr.empty() ) {
        if ( Lcurr.nRelocations() + Lcurr.LB() >= solBest->nRelocations() ) {
            break;
        }

        // if (verbose) {
        //     cout << endl << endl << "++++++++++++++++++++++++++++++++++++++++++++" << endl;
        //     cout << "Lcurr.nRelocations() + Lcurr.LB() = "
        //          << Lcurr.nRelocations() + Lcurr.LB() << endl;
        //     cout << "solBest->nRelocations() = " << solBest->nRelocations() << endl;
        // }
        
        tuple<int, int, int> relo = lookAheadAdvice(Lcurr, solBest);
        // cout << endl << "Look-ahead advice: " << get<0>(relo) << " --> " << get<1>(relo)
        //      << endl;

        // no advice!
        if ( get<0>(relo) == -1 ) {
            break;
        }
        
        Lcurr.relocate( get<0>(relo), get<1>(relo) );

        // exit(0);
        
        // cout << "Relocations: " << endl;
        // Lcurr.displaySolution();
        // cout << endl << endl;
        
        autoRetrieve( Lcurr );
    }
    return solBest;
}

// as in the article
// side-effect: solBest if updated if necessary
// (in the article, it is a global variable)
tuple<int, int, int> GLAH::lookAheadAdvice(const BRPState &state,
                                          shared_ptr<BRPState> &solBest) const {
    BRPState L0(state);
    return treeSearch(0, L0, solBest);
}

// format: <from, to, cost>
// side-effect: solBest if updated if necessary
// (in the article, it is a global variable)
tuple<int, int, int> GLAH::treeSearch(int d,
                                     const  BRPState &Ld,
                                     shared_ptr<BRPState> &solBest) const {
    
    // if (verbose) {
    //     for (int i=0; i < d; i++) { cout << "\t"; }
    //     cout << "-*-  ";
    // }
    
    // termination case 1
    if ( Ld.nRelocations() + Ld.LB() >= solBest->nRelocations() ) {
        
        // if (verbose) {
        //     cout << "Termination 1: unpromising" << endl;
        // }

        return make_tuple(-1, -1, -1);        
    } else if ( Ld.empty() || d == D_ ) { // termination case 2
        auto solEva = ubSolver_.solve(Ld);
        solEva->condenseJin();
        if ( solBest->nRelocations() > solEva->nRelocations() ) {
            solBest = solEva;

            // if (!verbose) {
            // cout << "New best solution: "
            //      << solEva->nRelocations() << " relocations" << endl;
            // }
            
        }

        // if (verbose) {
        //     cout << "Termination 2: condensed solution with "
        //          << solEva->nRelocations() << " relocations" << endl;
        // }
        
        return make_tuple(-1, -1, solEva->nRelocations() );
    } else { // general case

        // if (verbose) {
        //     cout << "General case: generating subtrees" << endl;
        // }
        
        int bestFrom=-1, bestTo=-1, bestCost=pow(Ld.n(), 2);
        // generate relocations here
        vector<pair<int, int> > reloList = genReloList(Ld);
        // now evaluate every child
        for (auto relo: reloList) {
            // cout << "Looking at relocation: " << relo.first << " --> "
            //      << relo.second << endl;
            BRPState Lnext(Ld);

            // if (verbose) {
            //     for (int i=0; i < d; i++) { cout << "\t"; }
            //     cout << "     " << relo.first << " --> "
            //          <<  relo.second << endl;
            // }
            
            Lnext.relocate(relo.first, relo.second);
            autoRetrieve(Lnext);
            auto child = treeSearch( d + 1, Lnext, solBest );
            if ( get<2>(child) != -1 && bestCost > get<2>(child) ) {

                // if (verbose) {
                //     for (int i=0; i < d; i++) { cout << "\t"; }
                //     cout << "     new best! old was " << bestCost
                //          << ", new is " << get<2>(child) << endl;
                // }

                bestFrom = relo.first;
                bestTo = relo.second;
                bestCost = get<2>(child);
            }
        }
        return make_tuple(bestFrom, bestTo, bestCost);
    }
}

// generate list of relocations for tree search
vector<pair<int, int> > GLAH::genReloList(const BRPState &state) const {

    // cout << "Current state:" << endl << state << endl << endl;
    
    
    // these tuples are of the form < score, sFrom, sTo > where the definition
    // of score varies depending on the type of relocation
    vector<tuple<int, int, int> >
        ftbgRelocs, nfbgRelocs, ftbbRelocs, nfbbRelocs, ggRelocs, gbRelocs;
    vector<pair<int, int> > finalRelocs;
    // add next relocation of evaluate()
    finalRelocs.push_back( ubSolver_.solveOnlyOne(state) );
    // sort all Relocations in different lists
    for (int s1=0; s1 < state.W(); s1++) {
        if ( state.height(s1) == 0 ) {
            continue;
        }
        for (int s2=0; s2 < state.W(); s2++) {
            if ( s1 == s2 or state.height(s2) == state.H() ) {
                continue;
            }
            // moving a well-placed item
            if ( state.top(s1) == state.low(s1) ) {
                // case 1: GG
                if ( state.top(s1) <= state.low(s2) ) {
                    // score multiplied by -1 to keep sorting in ascending order
                    ggRelocs.push_back( make_tuple( - state.f(s1)
                                                    + state.low(s2),
                                                    s1, s2 ) );
                } else { // case 2: GB
                    // score multiplied by -1 to keep sorting in ascending order
                    gbRelocs.push_back( make_tuple( - state.f(s1)
                                                    - state.low(s2),
                                                    s1, s2 ) );
                }
            } else { // moving a badly placed item
                // are we moving it to a well-placed location?
                if ( state.top(s1) <= state.low(s2) ) { // xx-BG relocations
                    int score = state.low(s2) - state.top(s1);
                    if ( state.low(s1) == state.next() ) { // case 3: FT-BG
                        ftbgRelocs.push_back( make_tuple( score, s1, s2 ) );
                    } else { // case 4: NF-BG
                        nfbgRelocs.push_back( make_tuple( score, s1, s2 ) );
                    }
                } else { // xx-BB relocations
                    int score = state.top(s1) - state.low(s2);
                    if ( state.low(s1) == state.next() ) { // case 5: FT-BB
                        ftbbRelocs.push_back( make_tuple( score, s1, s2 ) );
                    } else { // case 6: NF-BB
                        nfbbRelocs.push_back( make_tuple( score, s1, s2 ) );
                    }
                }
            }
        }
    }
    // used multiple times in the following
    int n;
    vector<tuple<int, int, int> >::const_iterator it;
    // FT-BG relocations
    // cout << "Considering " << ftbgRelocs.size() << " FT-BG relocations:"
    //      << endl;
    // for (auto r: ftbgRelocs) {
    //     cout << "\t" << get<0>(r) << "\t" << get<1>(r) << "\t" << get<2>(r)
    //          << endl;
    // }
    sort(ftbgRelocs.begin(), ftbgRelocs.end());
    n = min( ftbgRelocs.size(), nFTBG_ );
    for ( it = ftbgRelocs.begin(); it != ftbgRelocs.begin() + n; it++) {
        finalRelocs.push_back( make_pair( get<1>(*it), get<2>(*it) ) );
    }
    // NF-BG relocations
    // cout << "Considering " << nfbgRelocs.size() << " NF-BG relocations:"
    //      << endl;
    // for (auto r: nfbgRelocs) {
    //     cout << "\t" << get<0>(r) << "\t" << get<1>(r) << "\t" << get<2>(r)
    //          << endl;
    // }
    sort(nfbgRelocs.begin(), nfbgRelocs.end());
    n = min( nfbgRelocs.size(), nNFBG_ );
    for ( it = nfbgRelocs.begin(); it != nfbgRelocs.begin() + n; it++) {
        finalRelocs.push_back( make_pair( get<1>(*it), get<2>(*it) ) );
    }
    // FT-BB relocations
    // cout << "Considering " << ftbbRelocs.size() << " FT-BB relocations:"
    //      << endl;
    // for (auto r: ftbbRelocs) {
    //     cout << "\t" << get<0>(r) << "\t" << get<1>(r) << "\t" << get<2>(r)
    //          << endl;
    // }
    sort(ftbbRelocs.begin(), ftbbRelocs.end());
    n = min( ftbbRelocs.size(), nFTBB_ );
    for ( it = ftbbRelocs.begin(); it != ftbbRelocs.begin() + n; it++) {
        finalRelocs.push_back( make_pair( get<1>(*it), get<2>(*it) ) );
    }
    // NF-BB relocations
    // cout << "Considering " << nfbbRelocs.size() << " NF-BB relocations:"
    //      << endl;
    // for (auto r: nfbbRelocs) {
    //     cout << "\t" << get<0>(r) << "\t" << get<1>(r) << "\t" << get<2>(r)
    //          << endl;
    // }
    sort(nfbbRelocs.begin(), nfbbRelocs.end());
    n = min( nfbbRelocs.size(), nNFBB_ );
    for ( it = nfbbRelocs.begin(); it != nfbbRelocs.begin() + n; it++) {
        finalRelocs.push_back( make_pair( get<1>(*it), get<2>(*it) ) );
    }
    // GG relocations
    // cout << "Considering " << ggRelocs.size() << " GG relocations:"
    //      << endl;
    // for (auto r: ggRelocs) {
    //     cout << "\t" << get<0>(r) << "\t" << get<1>(r) << "\t" << get<2>(r)
    //          << endl;
    // }
    sort(ggRelocs.begin(), ggRelocs.end());
    n = min( ggRelocs.size(), nGG_ );
    for ( it = ggRelocs.begin(); it != ggRelocs.begin() + n; it++) {
        finalRelocs.push_back( make_pair( get<1>(*it), get<2>(*it) ) );
    }
    // GB relocations
    // cout << "Considering " << gbRelocs.size() << " GB relocations:"
    //      << endl;
    // for (auto r: gbRelocs) {
    //     cout << "\t" << get<0>(r) << "\t" << get<1>(r) << "\t" << get<2>(r)
    //          << endl;
    // }
    sort(gbRelocs.begin(), gbRelocs.end());
    n = min( gbRelocs.size(), nGB_ );
    for ( it = gbRelocs.begin(); it != gbRelocs.begin() + n; it++) {
        finalRelocs.push_back( make_pair( get<1>(*it), get<2>(*it) ) );
    }


    // cout << "Final set of relocations:" << endl;
    // for (auto it: finalRelocs) {
    //     cout << "\t" << it.first << " --> " << it.second << endl;
    // }
    
    // exit(8);
    return finalRelocs;
}
