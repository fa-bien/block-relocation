#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <iomanip>

#include "brpstate.h"
#include "branchandbound.h"
#include "genpolicy.h"

using namespace std;

// UB procedure for exact methods
unique_ptr<BRPPolicy> ubSolver;
// UB procedure for heuristic methods
unique_ptr<BRPPolicy> hubSolver;

bool verbose;

// for SmSEQC-X procedures
string condensationProcedure = "tricoire";

extern clock_t ticksInLB1;
extern clock_t ticksInLB2;
extern clock_t ticksInLB3;
extern map<int, long long> countForGap;
extern long long timeInLB1, timeInLB2, timeInLB3;

int main(int argc, char **argv) {

  string instanceFname = "instances/caserta/data3-3-1.dat";
  int seed = 0;
  string experiment = "branch-and-bound";
  string bbStrategy = "depth";
  string maxHeightType = "unlimited";//2H-1";
  int H = 3;
  int S = 3;
  string method = "LA-1";
  string ubMethod = "LA-1";
  string hubMethod = "FM";
  string scriptFile = "";
  int LB = 1;
  // string condensationProcedure = "none";
  verbose = false;
  bool debug = false;
  int timeLimit = 0;
  
  int i = 1;
  while (i<argc){
    string tmp = argv[i];
    if (tmp == "-i") {
      i++;
      instanceFname = argv[i];
      i++;
    } else if (tmp == "-maxHeight") {
      i++;
      maxHeightType = argv[i];
      i++;
    } else if (tmp == "-m") {
        i++;
        method = argv[i];
        i++;
    } else if (tmp == "-sf") {
        i++;
        scriptFile = argv[i];
        i++;
    } else if (tmp == "-ub") {
        i++;
        ubMethod = argv[i];
        i++;
    } else if (tmp == "-hub") {
        i++;
        hubMethod = argv[i];
        i++;
    } else if (tmp == "-lb") {
        i++;
        LB = atoi(argv[i]);
        i++;
    } else if (tmp == "-tl") {
      i++;
      timeLimit = atoi(argv[i]);
      i++;
    } else if (tmp == "-cp") {
      i++;
      condensationProcedure = argv[i];
      i++;
    } else if (tmp == "-bbs") {
      i++;
      bbStrategy = argv[i];
      i++;
    } else if (tmp == "-v") {
      i++;
      verbose = true;
    } else if (tmp == "-d") {
      i++;
      debug = true;
    } else {
      cerr << "unrecognized option: " << tmp << endl;
      exit(5);
    }
  }

  // dump parameter settings
  cout << "-----------------------------------------------------------" << endl;
  cout << "Parameter settings" << endl;
  cout << "-----------------------------------------------------------" << endl;
  cout << "Solution method:\t\t" << method << endl; 
  cout << "LB version:\t\t\t" << LB << endl; 
  cout << "BB strategy:\t\t\t" << bbStrategy << endl; 
  cout << "UB method:\t\t\t" << ubMethod << endl; 
  cout << "UB method for heuristics:\t" << hubMethod << endl;
  cout << "condensation procedure:\t\t" << condensationProcedure << endl;
  cout << "Instance file:\t\t\t" << instanceFname << endl;
  cout << "max. height:\t\t\t" << maxHeightType << endl;
  cout << "Time limit:\t\t\t" << timeLimit << endl;
  cout << "script file:\t\t\t" << scriptFile << endl;
  cout << "-----------------------------------------------------------" << endl;
  
  const BRPState s(instanceFname, maxHeightType);
  if ( verbose ) {
      cout << s << endl;
  }

  if ( scriptFile != "" ) {
      s.writeInstanceToFile(scriptFile);
  }

  // cout << "LB1 = " << s.LB1() << endl;
  // cout << "LB2 = " << s.LB2() << endl;
  // cout << "LB3 = " << s.LB3() << endl;
  
  cout << "initialising random number generator: srandom(" << seed << ");\n";
  srandom(seed);
  //
  BRPState::lbVersion = LB;
  // method used for UB calculation
  ubSolver = genPolicy(ubMethod, s, timeLimit, bbStrategy, true);
  hubSolver = genPolicy(hubMethod, s, timeLimit, bbStrategy, true);
  //
  // main solver we use
  auto solver = genPolicy(method, s, timeLimit, bbStrategy);
  clock_t ticksBefore = clock();
  shared_ptr<BRPState> result = solver->solve(s);
  clock_t ticksAfter = clock();
  cout << method << "\t used " << result->nRelocations() << " relocations in "
       << ( (double) (ticksAfter - ticksBefore) / CLOCKS_PER_SEC)
       << " s" << endl;
  
  if ( scriptFile != "" ) { 
      result->appendSolutionToFile(scriptFile);
  }
  
  if ( verbose ) {
      result->displaySolution();
  }

  if (LB == -1) {
      cout << "Time spent in LB1: " << ((double)ticksInLB1) / CLOCKS_PER_SEC
           << endl;
      cout << "Time spent in LB2: " << ((double)ticksInLB2) / CLOCKS_PER_SEC
           << endl;
      cout << "Time spent in LB3: " << ((double)ticksInLB3) / CLOCKS_PER_SEC
           << endl;
      cout << "/!\\ IMPORTANT NOTE: times are inaccurate"
           << " since a single call typically takes less than one tick"
           << endl;
      long long  nNodes = 0;
      for (auto it=countForGap.begin(); it != countForGap.end(); it++) {
          nNodes += it->second;
      }
      cout << nNodes << " nodes" << endl;
      cout.setf(ios::fixed);
      cout.precision(2);
      for (auto it=countForGap.begin(); it != countForGap.end(); it++) {
          cout << "gap = " << it->first << " :\t" << it->second << " nodes\t-\t"
               << 100.0 * it->second / nNodes << " %" << endl;
      }
  }
}
