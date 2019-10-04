The code is C++ and follows the C++14 standard. When using the Makefile, the
executable produced is called brp. Useful command line parameters:

-i <filename>:         Specify the input file
-sf <scriptfilename>:  Save the solution to a scriptfile
-v:                    Verbose mode (displays solution; by default, the solution
                       is not displayed)
-maxHeight <arg>:      Specify Hmax. Valid values are H+2, unlimited and 2H-1.
-m <algorithm>:        Specify which algorithm to use. Valid values are SM-1,
                       SM-2, SmSEQ-1, SmSEQ-2, JZW, LA-S-1, LA-<N>, GLAH-<N>,
                       RS-<N>, PM-<N>, DFBB, DFBB-L. <N> indicates a parameter
                       that is algorithm-specific; for instance to use rake
                       search with a width of 2, use RS-2.
-lb n:                 Lower bound to use in branch-and-bound.
                       Valid values are 1, 2, 3
-ub <algorithm>:       Algorithm to use to compute the initial lower bound for
                       branch-and-bound methods.
-hub <algorithm>:      Algorithm to use as an evaluation subroutine in the
                       pilot method.
-cp <procedure>:       Condensation procedure to use is SmSEQ-N algorithms.
                       Valid values are none, jin, tricoire.
-tl <limit>:           Time limit for branch-and-bound algorithms, in seconds.
