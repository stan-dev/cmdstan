pipeline {
    agent any
    options {
        disableConcurrentBuilds()
    }
    stages {
        stage('Clean & Setup') {
            steps {
                sh 'git clean -xffd'
                sh 'git submodule update --init --recursive'
                sh 'make stan-revert'
                sh "echo 'CC=${env.CXX}' > make/local"
                sh "echo 'CXXFLAGS += -Werror' >> make/local"
                sh "echo 'MAKEVARS=-j{env.PARALLEL}' >> make/local"
            }
        }
        stage('Interface tests') {
            steps { sh './runCmdStanTests.py src/test/interface' }
        }
        stage('Manual') {
            steps { sh 'make manual' }
        }
    }
    post {
        always {
            warnings consoleParsers: [[parserName: 'GNU C Compiler 4 (gcc)']], failedTotalAll: '0', usePreviousBuildAsReference: false, canRunOnFailed: true
            warnings consoleParsers: [[parserName: 'Clang (LLVM based)']], failedTotalAll: '0', usePreviousBuildAsReference: false, canRunOnFailed: true
        }
    }
}
