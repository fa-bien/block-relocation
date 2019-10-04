#include <iostream>
#include <fstream>
#include <algorithm>
#include <iomanip>
#include <map>

#include "brpstate.h"

int BRPState::lbVersion;

void BRPState::showOps() const {
    // cout << endl << endl << "OPs:" << endl;
    for (auto op: operations_) {
        cout << "\t(" << op.first << ", " << op.second << ")";
    }
    // cout << endl << "NR = " << nRelocations_ << endl
    //      << "LB = " << LB_ << endl;
    cout << endl;
}

BRPState::BRPState(string fName, string maxHeightType) {
    LB_ = 0;
    nRemaining_ = 0;
    lastRelocatedTo_ = -1;
    // W_, stacks_ and n_ are all initialised when reading the file
    readFromFile(fName);
    unsigned int h = *max_element(height_.begin(), height_.end());
    if (maxHeightType == "unlimited") {
        H_ = n_;
    } else if (maxHeightType == "H+2") {
        H_ = h + 2;
    } else if (maxHeightType == "2H-1") {
        H_ = 2 * h - 1;
    } else {
        cerr << "/!\\ Warning: max. height not set!!!" << endl;
        cerr << "--> using highest stack as limit!" << endl;
        H_ = h;
        cerr << "\tH = " << H_ << endl;
    }
    // meta-data section
    next_ = 1;
    nRelocations_ = 0;
    // must be set properly
    stackForItem_.resize(n_ + 1);
    for (unsigned int s=0; s < W_; s++) {
        for (vector<int>::const_iterator it = stacks_[s].begin();
             it != stacks_[s].end();
             it++) {
            stackForItem_[*it] = s;
        }
    }
}

// read an instance by Caserta et al.
void BRPState::readFromFile(string fName) {
    ifstream ifs;
    ifs.open(fName);
    ifs >> W_ >> n_;
    mustBeMoved_.resize(n_ + 1);
    stackForItem_.resize(n_ + 1);
    for (unsigned int i=0; i <= n_; i++) {
        mustBeMoved_[i] = false;
    }
    int thisH, tmp;
    for (unsigned int s=0; s < W_; s++) {
        stacks_.push_back(vector<int>());
        low_.push_back(n_ + 1);
        height_.push_back(0);
        ifs >> thisH;
        for (unsigned int j=0; j < thisH; j++) {
            ifs >> tmp;
            push(s, tmp);
        }
    }
    ifs.close();
}

void BRPState::relocate(int fromStack, int toStack) {
    int item = stacks_[fromStack].back();

    if ( height_[toStack] >= H_ ) {
        cerr << "Error: relocating " << item << " to stack " << toStack
             << " but it is full" << endl;
        exit(9);
    }
    
    // first, move item away from fromStack
    stacks_[fromStack].pop_back();
    height_[fromStack] -= 1;
    if (item == low_[fromStack]) {
        if (height_[fromStack] == 0) {
            low_[fromStack] = n_ + 1;
        } else {
            low_[fromStack] = *min_element( stacks_[fromStack].begin(),
                                            stacks_[fromStack].end() );
        }
    } else if (low_[fromStack] <= item) {
        LB_ -= 1;
    }
    // next, push it into toStack
    stacks_[toStack].push_back(item);
    height_[toStack] += 1;
    stackForItem_[item] = toStack;
    if (item < low_[toStack]) {
        low_[toStack] = item;
        mustBeMoved_[item] = false;
    } else {
        mustBeMoved_[item] = true;
        LB_ += 1;
    }
    // additional metadata
    nRelocations_ += 1;
    lastRelocatedTo_ = toStack;
    operations_.push_back(pair<int, int>(fromStack, toStack));
    // cout << "RELOCATE from " << fromStack << " to " << toStack << endl;
}

// only use to generate data, not to move items
void BRPState::push(int toStack, int item) {
    stacks_[toStack].push_back(item);
    stackForItem_[item] = toStack;
    nRemaining_ += 1;
    if (item < low_[toStack]) {
        low_[toStack] = item;
    } else {
        mustBeMoved_[item] = true;
        LB_ += 1;
    }
    height_[toStack] += 1;
}

// only use to retrieve items, not to move them
int BRPState::pop(int fromStack) {
    unsigned int item = stacks_[fromStack].back();
    stacks_[fromStack].pop_back();
    if (item != next_) {
        cerr << "Error: retrieving item " << item
             << " but the next to be retrieved is " << next_ << endl;
        exit(9);
    }
    stackForItem_[item] = -1;
    next_ = item + 1;
    nRemaining_ -= 1;
    height_[fromStack] -= 1;
    if (item == low_[fromStack]) {
        if (height_[fromStack] == 0) {
            low_[fromStack] = n_ + 1;
        } else {
            low_[fromStack] = *min_element( stacks_[fromStack].begin(),
                                            stacks_[fromStack].end() );
        }
    } else {
        LB_ -= 1;
    }
    operations_.push_back(pair<int, int>(fromStack, fromStack) );
    // meta data
    lastRelocatedTo_ = -1;
    //
    return item;
}

int BRPState::itemAt(int stack, int height) const {
    return stacks_[stack][height];
}

int BRPState::remainingSlots(int stack) {
    return H_ - height_[stack];
}

// return stacks for lowest a items
set<int> BRPState::stacksForLowestItems(int a) const {
    set<int> result;
    for (unsigned int i=next_; i < next_ + a; i++) {
        result.insert(stackForItem_[i]);
    }
    return result;
}

// items on top of specified stacks
vector<int> BRPState::tops(set<int> fromStacks) const {
    vector<int> result;
    for (auto s: fromStacks) {
        result.push_back(top(s));
    }
    return result;
}

bool BRPState::empty() const {
    return next_ > n_;
}

bool BRPState::dominates(const BRPState &other) const {
    return (stacks_ == other.stacks_) && (nRelocations_ <= other.nRelocations_);
}

// used to compare how promising is a state compared to another one
bool operator<(const BRPState &s1, const BRPState &s2) {
    return s1.LB() + s1.nRelocations() < s2.LB() + s2.nRelocations();
}

// used to compare how promising is a state compared to another one
bool operator<=(const BRPState &s1, const BRPState &s2) {
    return s1.LB() + s1.nRelocations() <= s2.LB() + s2.nRelocations();
}

bool operator==(const BRPState &s1, const BRPState &s2) {
    return (s1.LB() + s1.nRelocations() < s2.LB() + s2.nRelocations()) &&
        (s1.stacks() == s2.stacks());
}

ostream& operator<<(ostream &os, const BRPState &state) {
    for (unsigned int s=0; s < state.W(); s++) {
        os << " - " << s << " -";
    }
    os << endl;
    for (unsigned int i=0; i < 6 * state.W(); i++) {
        os << "_";
    }
    os << endl;
    for (int h=state.H()-1; h >= 0; h--) {
        for (unsigned int s=0; s < state.W(); s++) {
            if (state.height(s) > h) {
                os << " [" << setw(3) << setfill(' ') << state.itemAt(s, h)
                   << "]";
            } else {
                os << "      ";
            }
        }
        os << endl;
    }
    for (unsigned int i=0; i < 6 * state.W(); i++) {
        os << "*";
    }
    os << endl;
    // os << " lower bound = " << state.LB() + state.nRelocations() << endl;
    //
    for (unsigned int s=0; s < state.W(); s++) {
        os << " (" << setw(3) << setfill(' ') << state.low(s)
           << ")";
    }
    os << endl;
    return os;
}

bool BRPState::retrieveNext() {
    // cout << *this << endl;
    // cout << "next: " << next_ << endl;
    // cout << "stackForItem_[next]:" << stackForItem_[next_] << endl;
    // cout << "stacks_[stackForItem_[next]].back():"
    //      << stacks_[stackForItem_[next_]].back() << endl;
    if (next_ > n_) {
        return false;
    } else if (stacks_[stackForItem_[next_]].back() == next_) {
        // cout << "RETRIEVE " << next_ << endl;
        pop(stackForItem_[next_]);
        return true;
    } else {
        return false;
    }
}

// all necessary relocates
shared_ptr<vector<pair<int, int> > > BRPState::necessaryRelocates() const {
    auto result = make_shared<vector<pair<int, int> > >();
    for (int s=0; s < W(); s++) {
        if ( height_[s] > 1 ) {
            int item = top(s);
            for (int i=0; i < height_[s] - 1; i++) {
                // item on top of a smaller item: we need to relocate
                if (item > stacks_[s][i]) {
                    result->push_back(make_pair(s, item));
                    break;
                }
            }
        }
    }
    return result;
}

// all safe 1-relocates
shared_ptr<vector<tuple<int, int, int> > > BRPState::
safeRelocates(shared_ptr<vector<pair<int, int> > > relocates) const {
    if (relocates == NULL) {
        relocates = necessaryRelocates();
    }
    auto result = make_shared<vector<tuple<int, int, int> > >();
    for (auto reloc: *relocates) {
        for (unsigned int sTo = 0; sTo < W_; sTo++) {
            // only relocate to different stacks
            if (sTo != reloc.first && height_[sTo] < H_ &&
                low_[sTo] >= reloc.second) {
                result->push_back( make_tuple(reloc.first,
                                              sTo,
                                              low_[sTo] - reloc.second) );
            }
        }
    }
    return result;
}

// all safe 2-relocates
shared_ptr<vector<tuple<int, int, int> > > BRPState::
safe2Relocates(shared_ptr<vector<pair<int, int> > > relocates) const {
    if (relocates == NULL) {
        relocates = necessaryRelocates();
    }
    int highEnough = n_ << 2;
    auto result = make_shared<vector<tuple<int, int, int> > >();
    for (unsigned int sTo = 0; sTo < W_; sTo++) {
        int bestDiff = highEnough;
        int bestTo = -1;
        int bestToTo = -1;
        for (auto reloc: *relocates) {
            // only relocate to different stacks
            if (sTo != reloc.first && height_[sTo] > 0 &&
                top(sTo) < reloc.second) {
                int item3;
                if (height_[sTo] > 1) {
                    item3 = *(min_element( stacks_[sTo].begin(),
                                           stacks_[sTo].end()-1 ));
                } else {
                    item3 = n_ + 1;
                }
                // interesting case
                if (item3 >= reloc.second) {
                    vector<pair<int, int> > tmp = { make_pair(sTo, top(sTo)) };
                    auto sr =
                        bestSafeRelocate( make_shared<vector<
                                          pair<int, int> > > (tmp) );
                    // there exists a feasible safe 1-relocate
                    if (get<0>(sr) >= 0) {
                        // is it better than the best found so far for moving
                        // top(sTo)?
                        int diff = item3 - reloc.second + get<2>(sr);
                        if (diff < bestDiff) {
                            bestDiff = diff;
                            bestTo = sTo;
                            bestToTo = get<1>(sr);
                        }
                    }
                }
            }
        }
        if (bestTo > -1) {
            result->push_back( make_tuple( bestTo, bestToTo, bestDiff ) );
        }
    }
    return result;
}

tuple<int, int, int> BRPState::
bestSafeRelocate(shared_ptr<vector<pair<int, int> > > relocates) const {
    if (relocates == NULL) {
        relocates = necessaryRelocates();
    }
    tuple<int, int, int> best = make_tuple(-1, -1, n_ + 1);
    for (auto reloc: *relocates) {
        for (unsigned int sTo = 0; sTo < W_; sTo++) {
            // only relocate to different stacks
            if (sTo != reloc.first && height_[sTo] < H_ &&
                low_[sTo] >= reloc.second &&
                low_[sTo] - reloc.second < get<2>(best)) {
                get<0>(best) = reloc.first;
                get<1>(best) = sTo;
                get<2>(best) = low_[sTo] - reloc.second;
            }
        }
    }
    return best;
}

// tuple<int, int, int> BRPState::bestSafe2Relocate() const {
//     int highEnough = n_ << 2;
//     int bestFrom=-1, bestTo=-1, bestDiff=highEnough;
//     int item1, item2, item3;
//     for (unsigned int sFrom = 0; sFrom < W_; sFrom++) {
//         item1 = top(sFrom);
//         // only try to move items that are well-placed
//         if ( height_[sFrom] > 0 && item1 == low_[sFrom] ) {
//             // second smallest element in sFrom
//             int secondMin;
//             if (height_[sFrom] > 1) {
//                 secondMin = *(min_element( stacks_[sFrom].begin(),
//                                            stacks_[sFrom].end()-1 ));
//             } else {
//                 secondMin = n_ + 1;
//             }
//             for (unsigned int sTo = 0; sTo < W_; sTo++) {
//                 // only move to acceptable stacks
//                 if ( sTo != sFrom &&
//                      height_[sTo] < H_ &&
//                      item1 <= low_[sTo] ) {
//                     // look for safe 1-relocates induced by relocating
//                     // from sFrom to sTo
//                     for (unsigned int sExtra = 0; sExtra < W_; sExtra++) {
//                         item3 = top(sExtra);
//                         if ( sExtra != sFrom && sExtra != sTo &&
//                              height_[sExtra] > 1 &&
//                              item3 > low_[sExtra] &&
//                              item3 <= secondMin ) {
//                             int diff = low_[sTo] - item1
//                                 + secondMin - item3;
//                             if ( diff < bestDiff ) {
//                                 bestDiff = diff;
//                                 bestFrom = sFrom;
//                                 bestTo = sTo;
//                             }
//                         }
//                     }
//                 }
//             }
//         }
//     }
//     return make_tuple( bestFrom, bestTo, bestDiff );
// }

tuple<int, int, int> BRPState::bestSafe2Relocate() const {
    auto relocates = necessaryRelocates();
    int highEnough = n_ << 2;
    int bestDiff = highEnough;
    int bestTo = -1;
    int bestToTo = -1;
    for (unsigned int sTo = 0; sTo < W_; sTo++) {
        for (auto reloc: *relocates) {
            // only relocate to different stacks
            if (sTo != reloc.first && height_[sTo] > 0 &&
                top(sTo) < reloc.second) {
                int item3;
                if (height_[sTo] > 1) {
                    item3 = *(min_element( stacks_[sTo].begin(),
                                           stacks_[sTo].end()-1 ));
                } else {
                    item3 = n_ + 1;
                }
                // interesting case
                if (item3 >= reloc.second) {
                    vector<pair<int, int> > tmp = { make_pair(sTo, top(sTo)) };
                    auto sr =
                        bestSafeRelocate( make_shared<vector<
                                          pair<int, int> > > (tmp) );
                    // there exists a feasible safe 1-relocate
                    if (get<0>(sr) >= 0) {
                        // is it better than the best found so far for moving
                        // top(sTo)?
                        int diff = item3 - reloc.second + get<2>(sr);
                        if (diff < bestDiff) {
                            bestDiff = diff;
                            bestTo = sTo;
                            bestToTo = get<1>(sr);
                        }
                    }
                }
            }
        }
    }
    return make_tuple( bestTo, bestToTo, bestDiff );
}

// undo the last operation (retrieval or relocate)
// pre-condition: operations_ is not empty
// caveat: lastRelocatedTo_ is set to -1
void BRPState::undoLastMove() {
    pair<int, int> lastOp = operations_.back();
    operations_.pop_back();
    if ( lastOp.first == lastOp.second ) { // case 1: retrieval
        push( lastOp.first, next_ - 1);
        next_ -= 1;
        // cout << "UNDO: retrieval of " << next_ - 1 << endl;
    } else { // case 2: relocate
        relocate( lastOp.second, lastOp.first );
        nRelocations_ -= 2;
        operations_.pop_back();
        // cout << "UNDO: relocate from " << lastOp.first << " to "
        //      << lastOp.second << endl << "\t";
    }
}

void BRPState::displaySolution() const {
    int i = 0;
    for (auto op: operations_) {
        if (op.first == op.second) {
            cout << "Retrieve from " << op.first << "\t";
        } else {
            cout << "Relocate from " << op.first << " to " << op.second;
        }
        cout << "\t(" << i++ << ")" << endl;
    }
}

int BRPState::LB() const {
    switch (lbVersion) {
       case 1:
           return LB1();
       case 2:
           return LB2();
       case 3:
           return LB3();
       case -1:
           return LBcomp();
       default:
           return LB1();
    }
}

clock_t ticksInLB1 = 0;
clock_t ticksInLB2 = 0;
clock_t ticksInLB3 = 0;
map<int, long long> countForGap;

int BRPState::LBcomp() const {
    int lb1, lb2, lb3;
    clock_t before, after;
    //
    before = clock();
    lb1 = LB1();
    after = clock();
    ticksInLB1 += after - before;
    //
    before = clock();
    lb2 = LB2();
    after = clock();
    ticksInLB2 += after - before;
    //
    before = clock();
    lb3 = LB3();
    after = clock();
    ticksInLB3 += after - before;
    // cout << "LB1=" << lb1 << "\tLB2=" << lb2 << "\tLB3=" << lb3 << endl;
    int gap = lb3 - lb1;
    if ( countForGap.find(gap) == countForGap.end() ) {
        countForGap[gap] = 1;
    } else {
        countForGap[gap] += 1;
    }
    // if (lb1 != lb2) {
    //     cout << *this;
    //     cout << endl << "[return] to continue" << endl;
    //     getchar();
    // }
    return lb3;
}

int BRPState::LB1() const {
    return LB_;
}

int BRPState::LB2() const {
    int minTop = n_ + 1;
    int maxMin = 0;
    for (unsigned int s=0; s < W_; s++) {
        if ( height_[s] == 0 ) {
            return LB_;
        }
        int thisTop = top(s);
        if ( thisTop > low_[s] && thisTop < minTop ) {
            minTop = thisTop;
        }
        if ( low_[s] > maxMin ) {
            maxMin = low_[s];
        }
    }
    if ( minTop > maxMin ) {
        return LB_ + 1;
    } else {
        return LB_;
    }
}

int BRPState::LB3() const {
    // cout << "###############################" << endl;    
    // what is the height of the shortest stack?
    int shortestHeight = H_;
    for (unsigned int s=0; s < W_; s++) {
        if ( height_[s] < shortestHeight ) {
            shortestHeight = height_[s];
        }
    }
    // cout << "shortest height = " << shortestHeight << endl;
    int k = 0;
    while ( k < shortestHeight ) {
        int maxMin = 0;
        for (unsigned int s=0; s < W_; s++) {
            int tmp = lowestExceptTopK(s, k);
            if ( tmp > maxMin ) {
                maxMin = tmp;
            }
        }
        int minTop = n_ + 1;
        for (unsigned int s=0; s < W_; s++) {
            int thisTop = *(stacks_[s].rbegin() + k);
            if ( thisTop == next_ ) {
                // if we reach the next item to be retrieved, we stop
                minTop = thisTop;
                break;
            } else 
                if ( thisTop > low_[s] && thisTop < minTop ) {
                minTop = thisTop;
            }
        }
        // cout << "LB3: k = " << k << "\tminTop = " << minTop
        //      << "\tmaxMin = " << maxMin << endl;        
        if ( minTop > maxMin ) {
            k += 1;
        } else {
            break;
        }
    }
    return LB_ + k;
}

// minimum index of all items in stack s except its top k items
int BRPState::lowestExceptTopK(unsigned int s, unsigned int k) const {
    if ( k >= stacks_[s].size() ) {
        cerr << "Error in lowestExceptTopK: k = " << k
             << " but the stack only has "
             << stacks_[s].size() << " items" << endl;
        exit(22);
    }
    if ( k == 0 ) {
        return low_[s];
    } else {
        for ( unsigned int i=0; i < k; i++ ) {
            // case where the current min is within the k top elements:
            // compute the min of the remaining elements
            if ( *(stacks_[s].rbegin() + i) == low_[s] ) {
                return *(min_element( stacks_[s].begin(),
                                      stacks_[s].end()-k ));
            }
        }
        // if we reach this point, the current min is not within the top k items
        return low_[s];
    }
}

void BRPState::writeInstanceToFile(string fName) const {
    ofstream ofs;
    ofs.open(fName, ofstream::out);
    ofs << "instance BRPData(W=" << W_ << ", H=" << H_ << ", n=" << n_
        << ", stacks=[";
    for (auto s: stacks_) {
        ofs << "[";
        for (auto i: s) {
            ofs << i << ", ";
        }
        ofs << "], ";
    }
    ofs << "])" << endl;
    ofs.close();
}

void BRPState::appendSolutionToFile(string fName) const {
    ofstream ofs;
    ofs.open(fName, ofstream::out | ofstream::app);
    unsigned int t = 1;
    unsigned int i=0;
    for (auto op: operations_) {
        if (op.first == op.second) {
            ofs << "retrieving " << t++ << " from " << op.first << "\t";
        } else {
            ofs << "relocating x from " << op.first << " to " << op.second;
        }
        ofs << "\t(" << i++ << ")" << endl;
    }
    ofs.close();    
}

// minimum index of all items in stack s except its top item
int BRPState::f(unsigned int s) const{
    if (low_[s] == top(s)) {
        return *(min_element( stacks_[s].begin(),
                              stacks_[s].end()-1 ));
    } else {
        return low_[s];
    }
}

// minimum index of all items in stack s except its top item
int BRPState::smallestAbove(int c) const {
    int smallest = n_ + 1;
    auto it = stacks_[stackForItem_[c]].rbegin();
    while ( *it != c ) {
        if ( *it < smallest ) {
            smallest = *it;
        }
        ++it;
    }
    if (smallest == n_ + 1) {
        cerr << "Error in smallestAbove" << endl;
        exit(8);
    }
    return smallest;
}

void BRPState::condenseTricoire() {
    while(true) {
        int sizeBefore = nRelocations();
        condenseTricoireSub();
        if (sizeBefore == nRelocations()) {
            break;
        }
    }
}

void BRPState::condenseJin() {
    while(true) {
        int sizeBefore = nRelocations();
        condenseJinSub();
        if (sizeBefore == nRelocations()) {
            break;
        }
    }
}

void BRPState::condenseTricoireSub() {
    int firstReloc = 0;
    // compute height of each stack at each op
    vector<vector<int> > heightBeforeOp(operations_.size() + 1,
                                        vector<int>(W_, n_ / W_) );
    for (int op = 0; op < operations_.size(); op++) {
        int from = operations_[op].first;
        int to = operations_[op].second;
        for (int s=0; s < W_; s++) {
            if ( s == from ) {
                heightBeforeOp[op+1][s] = heightBeforeOp[op][s] - 1;
            } else if ( s == to && from != to ) {
                heightBeforeOp[op+1][s] = heightBeforeOp[op][s] + 1;
            } else {
                heightBeforeOp[op+1][s] = heightBeforeOp[op][s];
            }
        }
    }
    // now try to condense
    while (firstReloc < nRelocations_ - 1) {
        int s1 = operations_[firstReloc].first;
        int s2 = operations_[firstReloc].second;
        // special case: retrieval
        if (s1 == s2) {
            firstReloc += 1;
            continue;
        }
        int secondReloc = firstReloc + 1;
        while (secondReloc < nRelocations_) {            
            if (operations_[secondReloc].first == // it's a retrieval
                operations_[secondReloc].second) {
                secondReloc += 1;
                continue;
            }
            // can these two be possibly condensed?
            if ( s2 == operations_[secondReloc].first ) {
                int s3 = operations_[secondReloc].second;

                // cout << endl << endl;
                // cout << "firstReloc = " << firstReloc << endl;
                // cout << "secondReloc = " << secondReloc << endl;
                // cout << "s1 = " << s1 << endl;
                // cout << "s2 = " << s2 << endl;
                // cout << "s3 = " << s3 << endl << endl;
                
                // if ( nRelocations_ == 4825
                //      && firstReloc == 98
                //      && secondReloc > 125 ) {
                //     cout << "!!!" << endl;
                //     exit(9);
                // }
                
                // stack is full at this point
                if ( s1 != s3 && heightBeforeOp[firstReloc][s3] == H_ ) {
                    secondReloc += 1;

                    // cout << "STOP 1" << endl;
                    
                    continue;
                }
                // maybe they can!
                bool canBeCondensed = true;
                // we keep track of how many items are above the item
                // which has been relocated in firstReloc
                // we refer to this item as 'C'
                int aboveC = 0;
                int aboveS3 = 0;
                for ( int i = firstReloc + 1; i < secondReloc; i++ ) {
                    // special case 1: C is retrieved or moved
                    if ( aboveC == 0 &&
                         operations_[i].first == s2 ) {

                        // cout << "STOP 2" << endl;
                    
                        canBeCondensed = false;
                        break;
                    } else {
                        // special case 2: adding on top of C
                        if ( operations_[i].first != s2
                             && operations_[i].second == s2) {
                            aboveC += 1;
                        }
                        // special case 3: removing an item above C
                        if (operations_[i].first == s2) {
                            aboveC -= 1;
                            if (aboveC < 0) {
                                cerr << "This should not happen!" << endl;
                                exit(8);
                            }
                        }
                        // special case 4: retrieving an item from s3
                        if ( operations_[i].second == s3 ) {
                            if ( operations_[i].first == s3 ) {
                                canBeCondensed = false;

                                // cout << "STOP 3" << endl;
                    
                                break;
                            } else {
                                // special case 5: one more item on s3
                                aboveS3 += 1;
                                // if there was only one free spot it is also
                                // infeasible to condense, because this means
                                // s3 is being used as buffer stack and it
                                // won't be possible any more after condensation
                                if (aboveS3 + heightBeforeOp[i][s3] >= H_ - 1) {
                                    // case where condensing will cause
                                    // infeasibility due to height limit
                                    canBeCondensed = false;

                                    // cout << "STOP 4" << endl;
                    
                                    break;
                                }
                            }
                        }
                        // special case 6: one less item on final
                        // destination of C
                        if ( operations_[i].first == s3 ) {
                            aboveS3 -= 1;
                            if (aboveS3 < 0) {
                                canBeCondensed = false;

                                // cout << "STOP 5" << endl;
                    
                                break;
                            }
                        }
                    }
                }
                // final check: would a condensation make no difference?
                if ( aboveS3 != 0 || aboveC != 0 ) {

                    // cout << "STOP 6" << endl;
                        
                    canBeCondensed = false;
                }
                if (canBeCondensed) {
                    // condense ftw!
                    // cout << "condensing!!!" << endl;
                    // cout << "BEFORE: " << endl;
                    // displaySolution();
                    // cout << endl;
                    // cout << "firstReloc = " << firstReloc << endl;
                    // cout << "secondReloc = " << secondReloc << endl;
                    // cout << "s1 = " << s1 << endl;
                    // cout << "s2 = " << s2 << endl;
                    // cout << "s3 = " << s3 << endl << endl;
                    
                    // cout << "s1 = " << operations_[firstReloc].first << endl;
                    // cout << "s2 = " << operations_[firstReloc].second << endl;
                    // cout << "s2 = " << operations_[secondReloc].first << endl;
                    // cout << "s3 = " << operations_[secondReloc].second << endl;
                    for ( int i = firstReloc + 1; i < secondReloc; i++ ) {
                        heightBeforeOp[i][s2] -= 1;
                        heightBeforeOp[i][s3] += 1;
                    }
                    heightBeforeOp.erase(heightBeforeOp.begin() +
                                         secondReloc);
                    operations_.erase(operations_.begin() + secondReloc);
                    nRelocations_ -= 1;
                    operations_[firstReloc].second = s3;
                    s2 = s3;
                    // special case: moving back and forth
                    if ( s1 == s3 ) {
                        heightBeforeOp.erase(heightBeforeOp.begin() +
                                             firstReloc);
                        operations_.erase(operations_.begin() + firstReloc);
                        nRelocations_ -= 1;
                        // firstReloc += 1;
                        while (operations_[firstReloc].first ==
                               operations_[firstReloc].second) {
                            firstReloc += 1;
                        }
                        s1 = operations_[firstReloc].first;
                        s2 = operations_[firstReloc].second;
                        secondReloc = firstReloc + 1;
                    }
                    
                    // secondReloc = firstReloc;
                    // firstReloc = 0;

                    // cout << "AFTER:" << endl;
                    // displaySolution();
                    // cout << endl;
                    
                } else {
                    secondReloc += 1;
                }
            } else {
                secondReloc += 1;
            }
        }
        firstReloc += 1;
    }
}

void BRPState::condenseJinSub() {
    int firstReloc = 0;
    // now try to condense
    while (firstReloc < nRelocations_ - 1) {
        while (operations_[firstReloc].first ==
               operations_[firstReloc].second) {
            firstReloc += 1;
        }
        int s1 = operations_[firstReloc].first;
        int s2 = operations_[firstReloc].second;
        int secondReloc = firstReloc + 1;
        while (secondReloc < nRelocations_) {
            if ( operations_[secondReloc].first ==
                 operations_[secondReloc].second ) {
                secondReloc += 1;
                continue;
            }
            // can these two be possibly condensed?
            if ( operations_[firstReloc].second ==
                 operations_[secondReloc].first ) {
                int s3 = operations_[secondReloc].second;
                // maybe they can!
                bool canBeCondensed = true;
                // we keep track of how many items are above the item
                // which has been relocated in firstReloc
                // we refer to this item as 'C'
                int aboveC = 0;
                for ( int i = firstReloc + 1; i < secondReloc; i++ ) {
                    // special case 1: C is retrieved or moved
                    if ( aboveC == 0 &&
                         operations_[i].first == s2 ) {
                        canBeCondensed = false;
                        break;
                    } else {                        
                        // special case 2: adding on top of C
                        if ( operations_[i].first != s2
                             && operations_[i].second == s2 ) {
                            aboveC += 1;
                        }
                        // special case 3: removing an item above C
                        if (operations_[i].first == s2) {
                            aboveC -= 1;
                            if (aboveC < 0) {
                                cerr << "This should not happen!" << endl;
                                exit(8);
                            }
                        }
                        // special case 4: touching destination stack
                        if ( operations_[i].first == s3 ||
                             operations_[i].second == s3 ) {
                            canBeCondensed = false;
                            break;
                        }
                    }
                }
                // final check: is C still on top?
                if ( aboveC != 0 ) {
                    canBeCondensed = false;
                }
                if (canBeCondensed) {
                    // condense ftw!

                    // cout << "condensing!!!" << endl;
                    // // displaySolution();
                    // cout << "firstReloc = " << firstReloc << endl;
                    // cout << "secondReloc = " << secondReloc << endl;
                    // cout << "s1 = " << s1 << endl;
                    // cout << "s2 = " << s2 << endl;
                    // cout << "s3 = " << s3 << endl;
                    
                    // writeInstanceToFile("before.txt");
                    // appendSolutionToFile("before.txt");

                    operations_.erase(operations_.begin() + secondReloc);
                    nRelocations_ -= 1;
                    operations_[firstReloc].second = s3;
                    s2 = s3;
                    // special case: moving back and forth
                    if ( s1 == s3 ) {
                        operations_.erase(operations_.begin() + firstReloc);
                        nRelocations_ -= 1;
                        // firstReloc += 1;
                        while (operations_[firstReloc].first ==
                               operations_[firstReloc].second) {
                            firstReloc += 1;
                        }
                        s1 = operations_[firstReloc].first;
                        s2 = operations_[firstReloc].second;
                        secondReloc = firstReloc + 1;
                    }

                    // cout << "ops after condensation: " << endl;
                    // displaySolution();

                    // writeInstanceToFile("after.txt");
                    // appendSolutionToFile("after.txt");
                    // secondReloc = firstReloc;
                    // firstReloc = 0;
                } else {
                    secondReloc += 1;
                }
            } else {
                secondReloc += 1;
            }
        }
        firstReloc += 1;
    }
}
