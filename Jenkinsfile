@Library('StanUtils')
import org.stan.Utils

def utils = new org.stan.Utils()

def setupCXX(CXX = env.CXX) {
    unstash 'CmdStanSetup'
    writeFile(file: "make/local", text: "CXX = ${CXX}\n")
}

def runTests(String prefix = "") {
    """ make -j${env.PARALLEL} build
      ${prefix}runCmdStanTests.py -j${env.PARALLEL} src/test/interface
    """
}

def deleteDirWin() {
    bat "attrib -r -s /s /d"
    deleteDir()
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
                    post { 
                        always { 

                            recordIssues id: "Windows",
                            name: "Windows interface tests",
                            enabledForFailure: true,
                            aggregatingResults : true,
                            tools: [
                                gcc4(id: "Windows_gcc4", name: "Windows interface tests@GCC4"),
                                clang(id: "Windows_clang", name: "Windows interface tests@CLANG")
                            ],
                            blameDisabled: false,
                            qualityGates: [[threshold: 1, type: 'TOTAL', unstable: true]],
                            healthy: 10, unhealthy: 100, minimumSeverity: 'HIGH',
                            referenceJobName: env.BRANCH_NAME

                            deleteDirWin()
                        }
                    }
                }

                stage('Linux interface tests with MPI') {
                    agent {label 'linux && mpi'}
                    steps {
                        setupCXX("${MPICXX}")
                        sh "echo STAN_MPI=true >> make/local"
                        sh "make build-mpi > build-mpi.log 2>&1"
                        sh runTests("./")
                    }
                    post {
                        always {

                            recordIssues id: "Linux_mpi",
                            name: "Linux interface tests with MPI",
                            enabledForFailure: true,
                            aggregatingResults : true,
                            tools: [
                                gcc4(id: "Linux_mpi_gcc4", name: "Linux interface tests with MPI@GCC4"),
                                clang(id: "Linux_mpi_clang", name: "Linux interface tests with MPI@CLANG")
                            ],
                            blameDisabled: false,
                            qualityGates: [[threshold: 1, type: 'TOTAL', unstable: true]],
                            healthy: 10, unhealthy: 100, minimumSeverity: 'HIGH',
                            referenceJobName: env.BRANCH_NAME

                            deleteDir()
                        }
                    }
                }

                stage('Mac interface tests') {
                    agent {label 'osx'}
                    steps {
                        setupCXX()
                        sh runTests("./")
                    }
                    post {
                        always {

                            recordIssues id: "Mac",
                            name: "Mac interface tests",
                            enabledForFailure: true,
                            aggregatingResults : true,
                            tools: [
                                gcc4(id: "Mac_gcc4", name: "Mac interface tests@GCC4"),
                                clang(id: "Mac_clang", name: "Mac interface tests@CLANG")
                            ],
                            blameDisabled: false,
                            qualityGates: [[threshold: 1, type: 'TOTAL', unstable: true]],
                            healthy: 10, unhealthy: 100, minimumSeverity: 'HIGH',
                            referenceJobName: env.BRANCH_NAME

                            deleteDir()
                        }
                    }
                }

                stage('Upstream CmdStan Performance tests') {
                    when {
                            expression {
                                env.BRANCH_NAME ==~ /PR-\d+/ ||
                                env.BRANCH_NAME == "downstream_tests" ||
                                env.BRANCH_NAME == "downstream_hotfix"
                            }
                        }
                    steps {
                        script{
                            build(
                                job: "CmdStan Performance Tests/downstream_tests",
                                parameters: [
                                    string(name: 'cmdstan_pr', value: env.BRANCH_NAME),
                                    string(name: 'stan_pr', value: params.stan_pr),
                                    string(name: 'math_pr', value: params.math_pr)
                                ],
                                wait:false
                            )
                        }
                    }
                }

            }
        }
    }
    post {
        success { script { utils.mailBuildResults("SUCCESSFUL") } }
        unstable { script { utils.mailBuildResults("UNSTABLE", "stan-buildbot@googlegroups.com") } }
        failure { script { utils.mailBuildResults("FAILURE", "stan-buildbot@googlegroups.com") } }
    }
}
