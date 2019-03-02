@Library('StanUtils')
import org.stan.Utils

def utils = new org.stan.Utils()

def setupCXX(CXX = env.CXX) {
    unstash 'CmdStanSetup'
    writeFile(file: "make/local", text: "CXX = ${CXX}\n")
}

def runTests(String prefix = "") {
    """ make -j${env.PARALLEL} build
  ${prefix}runCmdStanTests.py src/test/interface
    """
}

pipeline {
    agent none
    options { skipDefaultCheckout() }
    parameters {
        string(defaultValue: '', name: 'stan_pr',
               description: "Stan PR to test against. Will check out this PR in the downstream Stan repo.")
        string(defaultValue: '', name: 'math_pr',
               description: "Math PR to test against. Will check out this PR in the downstream Math repo.")
    }
    stages {
        stage('Kill previous builds') {
            when {
                not { branch 'develop' }
                not { branch 'master' }
                not { branch 'downstream_tests' }
            }
            steps { script { utils.killOldBuilds() } }
        }
        stage('Clean & Setup') {
            agent any
            steps {
                retry(3) { checkout scm }
                sh 'git clean -xffd'
                sh 'make stan-revert'
                script {
                    utils.checkout_pr("stan", "stan", params.stan_pr)
                    utils.checkout_pr("math", "stan/lib/stan_math", params.math_pr)
                }
                stash 'CmdStanSetup'
            }
            post { always { deleteDir() }}
        }
        stage('Parallel tests') {
            parallel {
                stage('Windows interface tests') {
                    agent { label 'windows' }
                    steps {
                        setupCXX()
                        bat runTests()
                    }
                    post { always { deleteDir() }}
                }
                stage('Non-windows interface tests') {
                    agent any
                    steps {
                        setupCXX()
                        sh runTests("./")
                    }
                    post {
                        always {
                            warnings consoleParsers: [[parserName: 'GNU C Compiler 4 (gcc)']], failedTotalAll: '0', usePreviousBuildAsReference: false, canRunOnFailed: true
                            warnings consoleParsers: [[parserName: 'Clang (LLVM based)']], failedTotalAll: '0', usePreviousBuildAsReference: false, canRunOnFailed: true
                            deleteDir()
                        }
                    }
                }
                stage('Non-windows interface tests with MPI') {
                    agent { label 'linux' }
                    /* use system default compiler used to build MPI
                     environment {
                     OMPI_CXX = "${env.CXX}"
                     MPICH_CXX = "${env.CXX}"
                 }
                     */
                    steps {
                        setupCXX("${MPICXX}")
                        sh "echo STAN_MPI=true >> make/local"
                        sh "make build-mpi > build-mpi.log 2>&1"
                        sh runTests("./")
                    }
                    post {
                        always {
                            archiveArtifacts 'build-mpi.log'
                            warnings consoleParsers: [[parserName: 'GNU C Compiler 4 (gcc)']], failedTotalAll: '0', usePreviousBuildAsReference: false, canRunOnFailed: true
                            warnings consoleParsers: [[parserName: 'Clang (LLVM based)']], failedTotalAll: '0', usePreviousBuildAsReference: false, canRunOnFailed: true
                            deleteDir()
                        }
                        success { script { utils.mailBuildResults("SUCCESSFUL") } }
                        unstable { script { utils.mailBuildResults("UNSTABLE", "stan-buildbot@googlegroups.com") } }
                        failure { script { utils.mailBuildResults("FAILURE", "stan-buildbot@googlegroups.com") } }
                    }
                }
            }
        }
    }
}
