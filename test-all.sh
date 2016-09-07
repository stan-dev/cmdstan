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
pushd stan_2.12.0/
./runTests.py src/test
popd


echo ''
echo '------------------------------------------------------------'
echo 'Stan Math Library tests'
pushd stan_2.12.0/lib/stan_math_2.10.0/
./runTests.py test/unit
./runTests.py test/prob
popd
