#ifndef BRPSTATE_H
#define BRPSTATE_H

#include <vector>
#include <set>
#include <string>
#include <memory>
#include <iostream>

using namespace std;

class BRPState {
public:
    BRPState(string fName, string maxHeightType);

    void readFromFile(string fName);

    void writeInstanceToFile(string fName) const;
    void appendSolutionToFile(string fName) const;
    
    void relocate(int fromStack, int toStack);

    // only use to generate data, not to move items
    void push(int toStack, int item);

    // only use to retrieve items, not to move them
    int pop(int fromStack);

    int top(int fromStack) const { return stacks_[fromStack].back(); }

    vector<int>::reverse_iterator topIt(int fromStack) {
        return stacks_[fromStack].rbegin();
    }
    
    int itemAt(int stack, int height) const;

    int remainingSlots(int stack);

    // return stacks for lowest a items
    set<int> stacksForLowestItems(int a) const;

    unsigned int stackForItem(const int i) const { return stackForItem_[i]; }
    
    // items on top of specified stacks
    vector<int> tops(set<int> fromStacks) const;

    bool empty() const;

    int next() const {return next_; }

    int n() const {return n_; }
    
    bool dominates(const BRPState &other) const;

    int W() const { return W_; }
    
    int H() const { return H_; }

    int LB() const;
    int LB1() const;
    int LB2() const;
    int LB3() const;
    int LBcomp() const;

    // minimum index of all items in stack s except its top item
    int f(unsigned int s) const;
    
    // minimum index of all items in stack s except its top k items
    int lowestExceptTopK(unsigned int s, unsigned int k) const;

    // which LB are we using?
    static int lbVersion;
    
    int height(unsigned int s) const { return height_[s]; }

    int low(unsigned int s) const { return low_[s]; }

    unsigned int nRelocations() const { return nRelocations_; }

    unsigned int nRemaining() const { return nRemaining_; }

    bool mustBeMoved(unsigned int item) const { return mustBeMoved_[item]; }
    
    const vector<vector<int> > &stacks() const { return stacks_; }

    // attempts to retrieve the next item
    // returns true if successful, false otherwise
    bool retrieveNext();

    void setLastRelocatedTo(int s) { lastRelocatedTo_ = s; }

    void showOps() const;

    // undo the last operation (retrieval or relocate)
    // pre-condition: operations_ is not empty
    // caveat: lastRelocatedTo_ is set to -1
    void undoLastMove();
    
    // used in branch-and-bound
    // -1 means no stack
    int lastRelocatedTo() const { return lastRelocatedTo_; }

    // all necessary relocates
    shared_ptr<vector<pair<int, int> > > necessaryRelocates() const;
    
    // all safe 1-relocates
    shared_ptr<vector<tuple<int, int, int> > >
    safeRelocates(shared_ptr<vector<pair<int, int> > > relocates = NULL) const;
    
    // all safe 2-relocates
    shared_ptr<vector<tuple<int, int, int> > >
    safe2Relocates(shared_ptr<vector<pair<int, int> > > relocates = NULL) const;
    
    tuple<int, int, int>
    bestSafeRelocate(shared_ptr<vector<pair<int, int> > > relocates=NULL) const;
    
    tuple<int, int, int> bestSafe2Relocate() const;

    void displaySolution() const;

    // return the smallest element above i
    int smallestAbove(int i) const;

    // condense a solution, see Jin et al. (2015)
    void condenseJin();
    void condenseJinSub();
    // condense a solution, improved version
    void condenseTricoire();
    void condenseTricoireSub();
    
protected:
    int W_;
    int H_;
    int n_;
    int next_;
    vector<vector<int>> stacks_;
    int nRelocations_;
    vector<int> stackForItem_;
    unsigned int nRemaining_;
    vector<int> low_;
    vector<int> height_;
    vector<bool> mustBeMoved_;
    int LB_;
    // (from, to) pairs for relocations. (from, from) if it's a retrieval
    vector<pair<int, int>> operations_;
    // stack from where we retrieved an item last
    int lastRelocatedTo_;
};

// used to compare how promising is a state compared to another one
bool operator<(const BRPState &s1, const BRPState &s2);
// used to compare how promising is a state compared to another one
bool operator<=(const BRPState &s1, const BRPState &s2);
bool operator==(const BRPState &s1, const BRPState &s2);

ostream& operator<<(ostream &os, const BRPState &s2);

#endif
