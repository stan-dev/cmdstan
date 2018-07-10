@Library('StanUtils')
import org.stan.Utils

def utils = new org.stan.Utils()

def checkout_pr(String repo, String pr) {
    prNumber = pr.tokenize('-').last()
    if (repo == "math") {
        dir = "stan/lib/stan_math"
    } else {
        dir = repo
    }
    sh """
        cd ${dir}
        git clean -xffd
        git fetch https://github.com/stan-dev/${repo} +refs/pull/${prNumber}/merge:refs/remotes/origin/pr/${prNumber}/merge
        git checkout refs/remotes/origin/pr/${prNumber}/merge
    """
}

def setupCC(CC = env.CXX) {
    unstash 'CmdStanSetup'
    writeFile(file: "make/local", text: "CXX = ${CC}\n")
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
                    if (params.stan_pr != '') {
                        checkout_pr("stan", params.stan_pr)
                    }
                    if (params.math_pr != '') {
                        checkout_pr("math", params.math_pr)
                    }
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
                          setupCC()
                          bat runTests()
                    }
                    post { always { deleteDir() }}
                }
                stage('Non-windows interface tests') {
                    agent any
                    steps {
                          setupCC()
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
                          setupCC("${env.MPICXX}")
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
                    }
                }
                stage('Manual') {
                    agent any
                    steps {
                        unstash 'CmdStanSetup'
                        sh 'make manual'
                        archiveArtifacts 'doc/*'
                    }
                    post { always { deleteDir() }}
                }
            }
        }
    }
}
