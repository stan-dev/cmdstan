#!/usr/bin/python

"""
replacement for runtest target in Makefile
arg 1:  test dir or test file
"""

import os
import os.path
import platform
import sys
import subprocess
import time

WIN_SFX = '.exe'
TEST_SFX = '_test.cpp'
DEBUG = False
BATCH_SIZE = 25

def usage():
    sys.stdout.write('usage: %s <path/test/dir(/files)>\n' % sys.argv[0])
    sys.stdout.write('or\n')
    sys.stdout.write('       %s -j<#cores> <path/test/dir(/files)>\n' % sys.argv[0])
    sys.exit(0)

def stopErr(msg, returncode):
    sys.stderr.write('%s\n' % msg)
    sys.stderr.write('exit now (%s)\n' % time.strftime('%x %X %Z'))
    sys.exit(returncode)

def isWin():
    if (platform.system().lower().startswith('windows')
            or os.name.lower().startswith('windows')):
        return True
    return False

# edit filename into makefile target name
def mungeName(name):
    if DEBUG:
        print('munge input: %s' % name)
    if name.startswith('src/test/interface'):
        name = name.replace('src/','',1)
    if name.endswith(TEST_SFX):
        name, _ = os.path.splitext(name)
    if isWin():
        name += WIN_SFX
        name = name.replace('\\','/')

    name = name.replace(' ', '\\ ').replace('(','\\(').replace(')','\\)')
    if DEBUG:
        print('munge return: %s' % name)
    return name


def doCommand(command):
    print('------------------------------------------------------------')
    if isWin() and command.startswith('make '):
      command = command.replace('make ', 'mingw32-make ')
    print('%s' % command)
    p1 = subprocess.Popen(command,shell=True)
    p1.wait()
    if not (p1.returncode == None or p1.returncode == 0):
        stopErr('%s failed' % command, p1.returncode)

def makeTest(name, j):
    target = mungeName(name)
    if j == None:
        command = 'make %s' % target
    else:
        command = 'make -j%d %s' % (j,target)
    doCommand(command)

def makeBuild(j):
    if j == None:
        command = 'make build'
    else:
        command = 'make -j%d build' % j
    doCommand(command)


def makeTestModels(j):
    if j == None:
        command = 'make test-models-hpp'
    else:
        command = 'make -j%d test-models-hpp' % j
    doCommand(command)

def makeMathLibs(j):
    if j == None:
        command = 'make -f stan/lib/stan_math/make/standalone math-libs'
    else:
        command = 'make -j%d -f stan/lib/stan_math/make/standalone math-libs' % j
    doCommand(command)

def makeTests(dirname, filenames, j):
    targets = list()
    for name in filenames:
        if not name.endswith(TEST_SFX):
            continue
        target = '/'.join([dirname,name])
        target = mungeName(target)
        targets.append(target)
    if len(targets) > 0:
        if DEBUG:
            print('# targets: %d' % len(targets))
        startIdx = 0
        endIdx = BATCH_SIZE
        while (startIdx < len(targets)):
            if j == None:
                command = 'make %s' % ' '.join(targets[startIdx:endIdx])
            else:
                command = 'make -j%d %s' % (j,' '.join(targets[startIdx:endIdx]))
            if DEBUG:
                print('start %d, end %d' % (startIdx,endIdx))
                print(command)
            doCommand(command)
            startIdx = endIdx
            endIdx = startIdx + BATCH_SIZE
            if endIdx > len(targets):
                endIdx = len(targets)

def commandExists(command):
    p = subprocess.Popen(command, shell=True,
                         stdout=subprocess.PIPE,
                         stderr=subprocess.PIPE)
    p.wait()
    return p.returncode != 127

def runTest(name, mpi=False, j=1):
    executable = mungeName(name).replace('/',os.sep)
    xml = mungeName(name).replace(WIN_SFX, '')
    command = '%s --gtest_output="xml:%s.xml"' % (executable, xml)
    if mpi:
        if not commandExists('mpirun'):
            stopErr('Error: need to have mpi (and mpirun) installed to run mpi tests'
                    + '\nCheck https://github.com/stan-dev/stan/wiki/Parallelism-using-MPI-in-Stan for more details.'
                    , -1)
        if 'mpi_' in name:
            j = j > 2 and j or 2
            command = 'mpirun -np {} {}'.format(j, command)
    doCommand(command)

def main():
    if len(sys.argv) < 2:
        usage()

    try:
        with open('make/local') as f:
            stan_mpi =  'STAN_MPI' in f.read()
    except IOError:
        stan_mpi = False

    argsIdx = 1
    j = None
    if sys.argv[1].startswith('-j'):
        argsIdx = 2
        if len(sys.argv) < 3:
            usage()
        else:
            j = sys.argv[1].replace('-j','')
            try:
                jprime = int(j)
                if jprime < 1:
                    stopErr('bad value for -j flag',-1)
                j = jprime
            except ValueError:
                stopErr('bad value for -j flag',-1)


    # pass 0:  make build, compile all test models
    makeBuild(j)
    if (isWin()):
        makeMathLibs(j)
    makeTestModels(j)

    # pass 1:  call make to compile test targets
    for i in range(argsIdx,len(sys.argv)):
        testname = sys.argv[i]
        if DEBUG:
            print('testname: %s' % testname)
        if not os.path.exists(testname):
            stopErr('%s: no such file or directory' % testname,-1)
        if not os.path.isdir(testname):
            if not testname.endswith(TEST_SFX):
                stopErr('%s: not a testfile' % testname,-1)
            if DEBUG:
                print('make single test: %s' % testname)
            makeTest(testname,j)
        else:
            for root, dirs, files in os.walk(testname):
                if DEBUG:
                    print('make root: %s' % root)
                makeTests(root,files,j)

    # pass 2:  run test targets
    for i in range(argsIdx,len(sys.argv)):
        testname = sys.argv[i]
        if not os.path.isdir(testname):
            if DEBUG:
                print('run single test: %s' % testname)
            runTest(testname, mpi = stan_mpi, j = j)
        else:
            for root, dirs, files in os.walk(testname):
                for name in files:
                    if name.endswith(TEST_SFX):
                        if DEBUG:
                            print('run dir,test: %s,%s' % (root,name))
                        runTest(os.sep.join([root,name]), mpi = stan_mpi, j = j)


if __name__ == "__main__":
    main()
