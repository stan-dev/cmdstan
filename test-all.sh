#!/usr/bin/env bash

echo 'Running:'
echo '  - CmdStan tests'
echo '  - Stan tests'
echo '  - Stan Math Library tests'
echo ''
echo '------------------------------------------------------------'
echo 'CmdStan tests'
./runCmdStanTests.py src/test/interface
CMDSTAN_TESTS=$?

echo ''
echo '------------------------------------------------------------'
echo 'Stan tests'
pushd stan
./runTests.py src/test/unit
STAN_UNIT_TESTS=$?

./runTests.py src/test/integration
STAN_INTEGRATION_TESTS=$?

popd


echo ''
echo '------------------------------------------------------------'
echo 'Stan Math Library tests'
pushd stan/lib/stan_math_2.7.0
./runTests.py test/unit
MATH_UNIT_TESTS=$?

./runTests.py test/prob
MATH_PROB_TESTS=$?
popd


if [ $CMDSTAN_TESTS -eq 0 -a  $STAN_UNIT_TESTS -eq 0 -a  $STAN__INTEGRATION_TESTS -eq 0 -a $MATH_UNIT_TESTS -eq 0 -a  $MATH_PROB_TESTS -eq 0 ]; then
    RESULT="SUCCESS! ALL TESTS PASSED."
else
    RESULT="ERROR! SOME TESTS DID NOT PASS."
fi

cat <<EOF > TEST_RESULTS
Summary
Date:                 `date`
CmdStan directory: `basename $PWD`

CmdStan tests                 exit code = ${CMDSTAN_TESTS}
Stan unit tests               exit code = ${STAN_UNIT_TESTS}
Stan integration tests        exit code = ${STAN_INTEGRATION_TESTS}
Math unit tests               exit code = ${MATH_UNIT_TESTS}
Math distribution tests       exit code = ${MATH_PROB_TESTS}


${RESULT}
EOF

cat TEST_RESULTS

echo "Final report in $PWD/TEST_RESULTS"
