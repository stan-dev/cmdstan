#!/usr/bin/python

"""
Replacement for runtest target in Makefile.

Call script with '-h' as an option to see a helpful message.
"""

import sys
sys.path.append("./stan/lib/stan_math/")
from runTests import *

def makeTestModels(j):
    if j == None:
        command = 'make test-models-hpp'
    else:
        command = 'make -j%d test-models-hpp' % j
    doCommand(command)

def makeBuild(j):
    if j == None:
        command = 'make build'
    else:
        command = 'make -j%d build' % j
    doCommand(command)

def main():
    inputs = processCLIArgs()

    try:
        with open("make/local") as f:
            stan_mpi = "STAN_MPI" in f.read()
    except IOError:
        stan_mpi = False

    makeBuild(inputs.j)
    # pass 0: generate all auto-generated tests
    if any(['test-models' in arg for arg in inputs.tests]):
        makeTestModels(inputs.j)
        sys.exit()

    # If ./src/ is in front of call then just strip it out.
    tests = findTests(inputs.tests, inputs.f)
    tests = [arg.replace("src/", "") for arg in tests]

    if not tests:
        stopErr("No matching tests found.", -1)
    if inputs.debug:
        print("Collected the following tests:\n", tests)

    # pass 1: make test executables
    for batch in batched(tests):
        if inputs.debug:
            print("Test batch: ", batch)
        makeTest(" ".join(batch), inputs.j)

    if not inputs.make_only:
        # pass 2: run test targets
        for t in tests:
            if inputs.debug:
                print("run single test: %s" % testname)
            runTest(t, inputs.run_all, mpi=stan_mpi, j=inputs.j)


if __name__ == "__main__":
    main()
