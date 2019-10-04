#include <iostream>

#include "genpolicy.h"

using namespace std;

unique_ptr<BRPPolicy> genPolicy(string name,
                                const BRPState &s,
                                int timeLimit,
                                string bbStrategy,
                                bool mustBeHeuristic) {
    if (name == "LA-S-1") {
        return make_unique<LA_N>(s.W() - 1);
    } else if (name.substr(0, 3) == "LA-") {
        return make_unique<LA_N>(stoi(name.substr(3)));
    } else if (name.substr(0, 3) == "SM-") {
        return make_unique<SafeMovesPolicy>(stoi(name.substr(3)));
    } else if (name == "SSEQ") {
        return make_unique<SubsequencePolicy>(SubsequencePolicy());
    } else if (name.substr(0, 3) == "RS-") {
        return make_unique<RakeSearch>(stoi(name.substr(3)));
    } else if (name.size() > 5 && name.substr(0, 6) == "SmSEQ-") {
        return make_unique<SmartSubsequencePolicy>(stoi(name.substr(6)));
    } else if (name.substr(0, 3) == "PM-") {
        return make_unique<PilotMethod>(stoi(name.substr(3)));
    } else if (name == "JZW") {
        return make_unique<JZW>(JZW());
    } else if (name == "ZHU") {
        return make_unique<Zhu>(Zhu());
    } else if (name.substr(0, 5) == "GLAH-") {
        return make_unique<GLAH>(stoi(name.substr(5)));
    } else if (name == "FM") {
        return make_unique<FastMetaPolicy>(FastMetaPolicy());
        // exact methods follow
    } else if (name == "BB" && ! mustBeHeuristic) {
        return make_unique<BranchAndBound>(1e9, bbStrategy, timeLimit);
    } else if (name == "DFBB" && ! mustBeHeuristic) {
        return make_unique<DFBB>(1e9, timeLimit);
    } else if (name == "DFBB-L" && ! mustBeHeuristic) {
        return make_unique<DFBBLoop>(1e9, timeLimit);
    } else {
        cerr << "Invalid policy: " << name << endl;
        cerr << "mustBeHeuristic = " << mustBeHeuristic << endl;
        exit(22);
    }
}
