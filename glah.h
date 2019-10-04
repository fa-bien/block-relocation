#ifndef GLAH_H
#define GLAH_H

// depth-first branch-and-bound

#include <memory>

#include "brpstate.h"
#include "brppolicy.h"

class JZW: public BRPPolicy {
public:
    JZW() {}
    
    virtual string name() const { return "Jin et al. UB subroutine"; }
    
    virtual shared_ptr<BRPState> solve(const BRPState &initialState) const;
    // returns the next relocation that the heuristic would perform
    pair<int, int> solveOnlyOne(const BRPState &initialState) const;

protected:
    // as in the article
    virtual void gapUtilize(BRPState &state,
                            unsigned int s1, unsigned int s2) const;
    // returns the next relocation that gapUtilize would perform, or
    // <-1, -1> if it would not perform any
    pair<int, int> gapUtilizeOnlyOne(BRPState &state,
                                     unsigned int s1,
                                     unsigned int s2) const;
};

class Zhu: public JZW {
public:
    Zhu() {}
    
    virtual string name() const { return "Zhu's PU2"; }
    
protected:
    // as in the article
    virtual void gapUtilize(BRPState &state,
                            unsigned int s1, unsigned int s2) const {}
};

class GLAH: public BRPPolicy {
public:

    GLAH(unsigned int level = 3);
    
    virtual string name() const { return "Jin et al. GLAH method"; }
    
    virtual shared_ptr<BRPState> solve(const BRPState &initialState) const {
        return greedy(initialState);
    }
    
protected:
    // max recursion level in tree search
    unsigned int D_;

    JZW ubSolver_;

    // tree search parameters
    long unsigned int nFTBG_, nFTBB_, nNFBG_, nNFBB_, nGG_, nGB_;
    
    // used as UB
    shared_ptr<BRPState> evaluate(BRPState &state) const {
        return ubSolver_.solve(state);
    }

    // as in the article
    // side-effect: solBest if updated if necessary
    // (in the article, it is a global variable)
    tuple<int, int, int> lookAheadAdvice(const BRPState &state,
                                         shared_ptr<BRPState> &solBest) const;

    // as in the article
    // side-effect: solBest if updated if necessary
    // (in the article, it is a global variable)
    tuple<int, int, int> treeSearch(int d,
                                    const BRPState &state,
                                    shared_ptr<BRPState> &solBest) const;

    // wrapped by solve()
    shared_ptr<BRPState> greedy(const BRPState &initialState) const;

    // generate list of relocations for tree search
    vector<pair<int, int> > genReloList(const BRPState &state) const;
};

#endif
