#!/usr/bin/env bash

echo 'Running:'
echo '  - CmdStan tests'
echo '  - Stan tests'
echo '  - Stan Math Library tests'
echo ''
echo '------------------------------------------------------------'
echo 'CmdStan tests'
./runCmdStanTests.py src/test/interface


echo ''
echo '------------------------------------------------------------'
echo 'Stan tests'
pushd stan/
./runTests.py src/test
popd


echo ''
echo '------------------------------------------------------------'
echo 'Stan Math Library tests'
pushd stan/lib/stan_math_2.15.0/
./runTests.py test/unit
./runTests.py test/prob
popd
