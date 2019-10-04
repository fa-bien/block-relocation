#ifndef POLICIES_H
#define POLICIES_H

#include <string>
#include <memory>

#include "brpstate.h"
#include "brppolicy.h"
#include "petering.h"
#include "safemoves.h"
#include "subsequence.h"
#include "rakesearch.h"
#include "branchandbound.h"
#include "dfbb.h"
#include "pilotmethod.h"
#include "fastmeta.h"
#include "glah.h"

unique_ptr<BRPPolicy> genPolicy(string name,
                                const BRPState &s,
                                int timeLimit=0,
                                string bbStrategy="depth",
                                bool mustBeHeuristic=false);

#endif
