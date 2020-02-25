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

def runWinTests(String prefix = "") {
    withEnv(["PATH+TBB=${WORKSPACE}\\stan\\lib\\stan_math\\lib\\tbb"]) {
       bat "echo %PATH%"
       bat "mingw32-make -j${env.PARALLEL} build"
       bat "${prefix}runCmdStanTests.py -j${env.PARALLEL} src/test/interface"
    }
}

def deleteDirWin() {
    bat "attrib -r -s /s /d"
    deleteDir()
}

def sourceCodePaths(){
    // These paths will be passed to git diff
    // If there are changes to them, CI/CD will continue else skip
    def paths = ['src/cmdstan', 'src/test', 'lib', 'examples', 'make', 'stan', 'install-tbb.bat', 'makefile', 'runCmdStanTests.py', 'test-all.sh', 'Jenkinsfile']
    def bashArray = ""

    for(path in paths){
        bashArray += path + (path != paths[paths.size() - 1] ? " " : "")
    }

    return bashArray
}

def skipRemainingStages = true

pipeline {
    agent none
    options { skipDefaultCheckout() }
    environment {
        scPaths = sourceCodePaths()
    }
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
        stage('Verify changes') {
            agent { label 'linux' }
            steps {
                script {         

                    retry(3) { checkout scm }
                    sh 'git clean -xffd'

                    def commitHash = sh(script: "git rev-parse HEAD | tr '\\n' ' '", returnStdout: true)
                    def changeTarget = ""

                    if (env.CHANGE_TARGET) {
                        println "This build is a PR, checking out target branch to compare changes."
                        changeTarget = env.CHANGE_TARGET
                        sh(script: "git pull && git checkout ${changeTarget}", returnStdout: false)
                    }
                    else{
                        println "This build is not PR, checking out current branch and extract HEAD^1 commit to compare changes or develop when downstream_tests."
                        if (env.BRANCH_NAME == "downstream_tests"){
                            sh(script: "git checkout develop && git pull", returnStdout: false)
                            changeTarget = sh(script: "git rev-parse HEAD^1 | tr '\\n' ' '", returnStdout: true)
                            sh(script: "git checkout ${commitHash}", returnStdout: false)
                        }
                        else{
                            sh(script: "git pull && git checkout ${env.BRANCH_NAME}", returnStdout: false)
                            changeTarget = sh(script: "git rev-parse HEAD^1 | tr '\\n' ' '", returnStdout: true)
                        }
                    }

                    println "Comparing differences between current ${commitHash} and target ${changeTarget}."

                    def bashScript = """
                        for i in ${env.scPaths};
                        do
                            git diff ${commitHash} ${changeTarget} -- \$i
                        done
                    """

                    def differences = sh(script: bashScript, returnStdout: true)

                    println differences

                    if (differences?.trim()) {
                        println "There are differences in the source code, CI/CD will run."
                        skipRemainingStages = false
                    }
                    else{
                        println "There aren't any differences in the source code, CI/CD will not run."
                        skipRemainingStages = true
                    }
                }
            }
            post {
                always {
                    deleteDir()
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
                        setupCXX()
                        runWinTests()
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
                    agent { label 'linux && mpi'}
                    steps {
                        setupCXX("${MPICXX}")
                        sh "echo STAN_MPI=true >> make/local"
                        sh "echo CXX_TYPE=gcc >> make/local"
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
                    agent { label 'osx'}
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
        success {
           script {
               if (env.BRANCH_NAME == "develop") {
                   build job: "CmdStan Performance Tests/master", wait:false
               }
               utils.mailBuildResults("SUCCESSFUL")
           }
        }
        unstable { script { utils.mailBuildResults("UNSTABLE", "stan-buildbot@googlegroups.com") } }
        failure { script { utils.mailBuildResults("FAILURE", "stan-buildbot@googlegroups.com") } }
    }
}
