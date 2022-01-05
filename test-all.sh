#!/usr/bin/env bash

if [ "$#" -ne 1 ]; then
RUNTESTARGS=""
else
RUNTESTARGS="-j"$1
fi

echo 'Running:'
echo '  - CmdStan tests'
echo '  - Stan tests'
echo '  - Stan Math Library tests'
echo ''
echo '------------------------------------------------------------'
echo 'CmdStan tests'
./runCmdStanTests.py $RUNTESTARGS src/test/interface


echo ''
echo '------------------------------------------------------------'
echo 'Stan tests'
pushd stan/
./runTests.py $RUNTESTARGS src/test
popd


echo ''
echo '------------------------------------------------------------'
echo 'Stan Math Library tests'
pushd stan/lib/stan_math/
./runTests.py $RUNTESTARGS test/unit
./runTests.py $RUNTESTARGS test/prob
popd
