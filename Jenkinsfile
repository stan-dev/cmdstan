@Library('StanUtils')
import org.stan.Utils

def utils = new org.stan.Utils()
def skipRemainingStages = false

def setupCXX(CXX = env.CXX) {
    unstash 'CmdStanSetup'

    stanc3_bin_url_str = params.stanc3_bin_url != "nightly" ? "\nSTANC3_TEST_BIN_URL=${params.stanc3_bin_url}\n" : ""
    writeFile(file: "make/local", text: "CXX=${CXX} \n${stanc3_bin_url_str} \nCXXFLAGS+=-Wp,-D_GLIBCXX_ASSERTIONS\n")
}

def runTests(String prefix = "") {
    """ make -j${env.PARALLEL} build
        ${prefix}runCmdStanTests.py -j${env.PARALLEL} src/test/interface
    """
}

def runWinTests(String prefix = "") {
    withEnv(["PATH+TBB=${WORKSPACE}\\stan\\lib\\stan_math\\lib\\tbb"]) {
        bat """
            SET \"PATH=${env.RTOOLS40_HOME};%PATH%\"
            SET \"PATH=${env.RTOOLS40_HOME}\\usr\\bin;${LLVM7}\\bin;%PATH%\" //
            SET \"PATH=${env.RTOOLS40_HOME}\\mingw64\\bin;%PATH%\"
            SET \"PATH=C:\\PROGRA~1\\R\\R-4.1.2\\bin;%PATH%\"
            SET \"PATH=C:\\PROGRA~1\\Microsoft^ MPI\\Bin;%PATH%\"
            SET \"MPI_HOME=C:\\PROGRA~1\\Microsoft^ MPI\\Bin\"
            SET \"PATH=C:\\Users\\jenkins\\Anaconda3;%PATH%\"
            make -j${env.PARALLEL} build
            python ${prefix}runCmdStanTests.py -j${env.PARALLEL} src/test/interface
        """
    }
}

def deleteDirWin() {
    bat "attrib -r -s /s /d"
    deleteDir()
}

def isBranch(String b) { env.BRANCH_NAME == b }
Boolean isPR() { env.CHANGE_URL != null }
String fork() { env.CHANGE_FORK ?: "stan-dev" }
String branchName() { isPR() ? env.CHANGE_BRANCH :env.BRANCH_NAME }


pipeline {
    agent none
    options {
        skipDefaultCheckout()
        disableConcurrentBuilds(abortPrevious: env.BRANCH_NAME != "downstream_tests" && env.BRANCH_NAME != "downstream_hotfix")
    }
    parameters {
        string(defaultValue: '', name: 'stan_pr',
               description: "Stan PR to test against. Will check out this PR in the downstream Stan repo.")
        string(defaultValue: '', name: 'math_pr',
               description: "Math PR to test against. Will check out this PR in the downstream Math repo.")
        string(defaultValue: 'nightly', name: 'stanc3_bin_url',
                  description: 'Custom stanc3 binary url')
    }
    environment {
        MAC_CXX = 'clang++'
        LINUX_CXX = 'clang++-6.0'
        WIN_CXX = 'g++'
        PARALLEL = 8
        MPICXX = 'mpicxx.openmpi'
        GIT_AUTHOR_NAME = 'Stan Jenkins'
        GIT_AUTHOR_EMAIL = 'mc.stanislaw@gmail.com'
        GIT_COMMITTER_NAME = 'Stan Jenkins'
        GIT_COMMITTER_EMAIL = 'mc.stanislaw@gmail.com'
    }
    stages {
        stage('Clean & Setup') {
            agent {
                docker {
                    image 'stanorg/ci:gpu'
                    label 'linux'
                }
            }
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
        stage('Verify changes') {
            agent {
                docker {
                    image 'stanorg/ci:gpu'
                    label 'linux'
                }
            }
            steps {
                script {
                    retry(3) { checkout scm }
                    sh 'git clean -xffd'

                    def paths = ['src/cmdstan', 'src/test', 'lib', 'examples', 'make', 'stan', 'install-tbb.bat', 'makefile', 'runCmdStanTests.py', 'test-all.sh', 'Jenkinsfile'].join(" ")
                    skipRemainingStages = utils.verifyChanges(paths)
                }
            }
            post {
                always {
                    deleteDir()
                }
            }
        }
        stage("Clang-format") {
            agent {
                docker {
                    image 'stanorg/ci:gpu'
                    label 'linux'
                }
            }
            steps {
                retry(3) { checkout scm }
                withCredentials([usernamePassword(credentialsId: 'a630aebc-6861-4e69-b497-fd7f496ec46b',
                    usernameVariable: 'GIT_USERNAME', passwordVariable: 'GIT_PASSWORD')]) {
                    sh """#!/bin/bash
                        set -x
                        git checkout -b ${branchName()}
                        clang-format --version
                        find src -name '*.hpp' -o -name '*.cpp' | xargs -n20 -P${env.PARALLEL} clang-format -i
                        if [[ `git diff` != "" ]]; then
                            git add src
                            git commit -m "[Jenkins] auto-formatting by `clang-format --version`"
                            git push https://${GIT_USERNAME}:${GIT_PASSWORD}@github.com/${fork()}/cmdstan.git ${branchName()}
                            echo "Exiting build because clang-format found changes."
                            echo "Those changes are now found on stan-dev/cmdstan under branch ${branchName()}"
                            echo "Please 'git pull' before continuing to develop."
                            exit 1
                        fi
                    """
                }
            }
            post {
                always { deleteDir() }
                failure {
                    script {
                        emailext (
                            subject: "[StanJenkins] Autoformattted: Job '${env.JOB_NAME} [${env.BUILD_NUMBER}]'",
                            body: "Job '${env.JOB_NAME} [${env.BUILD_NUMBER}]' " +
                                "has been autoformatted and the changes committed " +
                                "to your branch, if permissions allowed." +
                                "Please pull these changes before continuing." +
                                "\n\n" +
                                "See https://github.com/stan-dev/stan/wiki/Coding-Style-and-Idioms" +
                                " for setting up the autoformatter locally.\n"+
                            "(Check console output at ${env.BUILD_URL})",
                            recipientProviders: [[$class: 'RequesterRecipientProvider']],
                            to: "${env.CHANGE_AUTHOR_EMAIL}"
                        )
                    }
                }
            }
        }
        stage('Parallel tests') {
            when {
                expression {
                    !skipRemainingStages
                }
            }
            parallel {
                stage('Windows interface tests') {
                    agent { label 'windows' }
                    steps {
                        setupCXX(WIN_CXX)
                        runWinTests()
                    }
                    post {
                        always {

                            recordIssues(
                                id: "Windows",
                                name: "Windows interface tests",
                                enabledForFailure: true,
                                aggregatingResults : false,
                                filters: [
                                    excludeFile('/lib/.*'),
                                    excludeFile('tbb/*'),
                                    excludeFile('stan/lib/stan_math/lib/*'),
                                    excludeMessage(".*'sprintf' is deprecated.*")
                                ],
                                tools: [
                                    gcc4(id: "Windows_gcc4", name: "Windows interface tests@GCC4"),
                                    clang(id: "Windows_clang", name: "Windows interface tests@CLANG")
                                ],
                                qualityGates: [[threshold: 1, type: 'TOTAL', unstable: true]],
                                healthy: 10, unhealthy: 100, minimumSeverity: 'HIGH'
                            )

                            deleteDirWin()
                        }
                    }
                }

                stage('Linux interface tests with MPI') {
                    agent {
                        docker {
                            image 'stanorg/ci:gpu'
                            label 'linux'
                        }
                    }
                    steps {
                        setupCXX(MPICXX)
                        sh "echo STAN_MPI=true >> make/local"
                        sh "echo CXX_TYPE=gcc >> make/local"
                        sh "make build-mpi > build-mpi.log 2>&1"
                        sh runTests("./")
                    }
                    post {
                        always {

                            recordIssues(
                                id: "Linux_mpi",
                                name: "Linux interface tests with MPI",
                                enabledForFailure: true,
                                aggregatingResults : false,
                                filters: [
                                    excludeFile('/lib/.*'),
                                    excludeFile('tbb/*'),
                                    excludeFile('stan/lib/stan_math/lib/*'),
                                    excludeMessage(".*'sprintf' is deprecated.*")
                                ],
                                tools: [
                                    gcc4(id: "Linux_mpi_gcc4", name: "Linux interface tests with MPI@GCC4"),
                                    clang(id: "Linux_mpi_clang", name: "Linux interface tests with MPI@CLANG")
                                ],
                                qualityGates: [[threshold: 1, type: 'TOTAL', unstable: true]],
                                healthy: 10, unhealthy: 100, minimumSeverity: 'HIGH'
                            )

                            deleteDir()
                        }
                    }
                }

                stage('Mac interface tests') {
                    agent { label 'osx' }
                    steps {
                        setupCXX(MAC_CXX)
                        sh runTests("python3 ./")
                    }
                    post {
                        always {

                            recordIssues(
                                id: "Mac",
                                name: "Mac interface tests",
                                enabledForFailure: true,
                                aggregatingResults : false,
                                filters: [
                                    excludeFile('/lib/.*'),
                                    excludeFile('tbb/*'),
                                    excludeFile('stan/lib/stan_math/lib/*'),
                                    excludeMessage(".*'sprintf' is deprecated.*")
                                ],
                                tools: [
                                    gcc4(id: "Mac_gcc4", name: "Mac interface tests@GCC4"),
                                    clang(id: "Mac_clang", name: "Mac interface tests@CLANG")
                                ],
                                qualityGates: [[threshold: 1, type: 'TOTAL', unstable: true]],
                                healthy: 10, unhealthy: 100, minimumSeverity: 'HIGH'
                            )

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
                                job: "Stan/CmdStan Performance Tests/downstream_tests",
                                parameters: [
                                    string(name: 'cmdstan_pr', value: env.BRANCH_NAME),
                                    string(name: 'stan_pr', value: params.stan_pr),
                                    string(name: 'math_pr', value: params.math_pr),
                                    string(name: 'stanc3_bin_url', value: params.stanc3_bin_url)
                                ],
                                wait:true
                            )
                        }
                    }
                }

            }
        }
        stage('Update downstream branches') {
            parallel {
                stage('Update downstream_hotfix - master') {
                    agent {
                        docker {
                            image 'ellerbrock/alpine-bash-git'
                            label 'linux'
                            args '--entrypoint='
                        }
                    }
                    when {
                        beforeAgent true
                        branch 'master'
                    }
                    steps {
                        script {
                            retry(3) {
                                checkout([
                                    $class: 'GitSCM',
                                    branches: [[name: '*/master'], [name: '*/downstream_hotfix']],
                                    userRemoteConfigs: scm.userRemoteConfigs
                                ])
                             }
                            withCredentials([usernamePassword(credentialsId: 'a630aebc-6861-4e69-b497-fd7f496ec46b',
                                usernameVariable: 'GIT_USERNAME', passwordVariable: 'GIT_PASSWORD')]) {
                                sh """#!/bin/bash
                                    set -x

                                    git checkout downstream_hotfix
                                    git reset --hard origin/master
                                    git status
                                    git push -f https://${GIT_USERNAME}:${GIT_PASSWORD}@github.com/stan-dev/cmdstan.git downstream_hotfix
                                """
                            }
                        }
                    }
                    post {
                        always {
                            deleteDir()
                        }
                    }
                }
                stage('Update downstream_tests - develop') {
                    agent {
                        docker {
                            image 'ellerbrock/alpine-bash-git'
                            label 'linux'
                            args '--entrypoint='
                        }
                    }
                    when {
                        beforeAgent true
                        branch 'develop'
                    }
                    steps {
                        script {
                            retry(3) {
                                checkout([
                                    $class: 'GitSCM',
                                    branches: [[name: '*/develop'], [name: '*/downstream_tests']],
                                    userRemoteConfigs: scm.userRemoteConfigs
                                ])
                             }
                            withCredentials([usernamePassword(credentialsId: 'a630aebc-6861-4e69-b497-fd7f496ec46b',
                                usernameVariable: 'GIT_USERNAME', passwordVariable: 'GIT_PASSWORD')]) {
                                sh """#!/bin/bash
                                    set -x

                                    git checkout downstream_tests
                                    git reset --hard origin/develop
                                    git status
                                    git push https://${GIT_USERNAME}:${GIT_PASSWORD}@github.com/stan-dev/cmdstan.git downstream_tests
                                """
                            }
                        }
                    }
                    post {
                        always {
                            deleteDir()
                        }
                    }
                }
            }
        }
    }
    post {
        success {
           script {
               if (env.BRANCH_NAME == "develop") {
                   build job: "Stan/CmdStan Performance Tests/master", wait:false
               }
               utils.mailBuildResults("SUCCESSFUL")
           }
        }
        unstable { script { utils.mailBuildResults("UNSTABLE", "stan-buildbot@googlegroups.com") } }
        failure { script { utils.mailBuildResults("FAILURE", "stan-buildbot@googlegroups.com") } }
    }
}
