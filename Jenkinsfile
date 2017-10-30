def checkout_pr(String repo, String pr) {
    prNumber = pr.tokenize('-').last()
    if (repo == "math") {
        dir = "stan/lib/stan_math"
    } else {
        dir = repo
    }
    sh """
        cd ${dir}
        git fetch https://github.com/stan-dev/${repo} +refs/pull/${prNumber}/merge:refs/remotes/origin/pr/${prNumber}/merge
        git checkout refs/remotes/origin/pr/${prNumber}/merge
    """
}

pipeline {
    agent any
    options {
        disableConcurrentBuilds()
    }
    parameters {
        string(defaultValue: '', name: 'stan_pr',
          description: "Stan PR to test against. Will check out this PR in the downstream Stan repo.")
        string(defaultValue: '', name: 'math_pr',
          description: "Math PR to test against. Will check out this PR in the downstream Math repo.")
    }
    stages {
        stage('Clean & Setup') {
            steps {
                sh 'git clean -xffd'
                sh 'git submodule update --init --recursive'
                sh 'make stan-revert'
                script {
                    if (params.stan_pr != '') {
                        checkout_pr("stan", params.stan_pr)
                    }
                    if (params.math_pr != '') {
                        checkout_pr("math", params.math_pr)
                    }
                }
                sh "echo 'CC=${env.CXX}' > make/local"
                sh "echo 'CXXFLAGS += -Werror' >> make/local"
                sh "echo 'MAKEVARS=-j{env.PARALLEL}' >> make/local"
            }
        }
        stage('Interface tests') {
            steps { sh './runCmdStanTests.py src/test/interface' }
        }
        stage('Manual') {
            steps {
                sh 'make manual'
                archiveArtifacts 'doc/*'
            }
        }
    }
    post {
        always {
            warnings consoleParsers: [[parserName: 'GNU C Compiler 4 (gcc)']], failedTotalAll: '0', usePreviousBuildAsReference: false, canRunOnFailed: true
            warnings consoleParsers: [[parserName: 'Clang (LLVM based)']], failedTotalAll: '0', usePreviousBuildAsReference: false, canRunOnFailed: true
            deleteDir()
        }
    }
}
