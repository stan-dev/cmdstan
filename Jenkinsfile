properties([
  disableConcurrentBuilds(abortPrevious: env.BRANCH_NAME != "downstream_tests" && env.BRANCH_NAME != "downstream_hotfix"),
  buildDiscarder(logRotator(numToKeepStr: '20', daysToKeepStr: '30')),
  parameters([
    string(defaultValue: '', name: 'stan_pr',
           description: "Stan PR to test against. Will check out this PR in the downstream Stan repo."),
    string(defaultValue: '', name: 'math_pr',
           description: "Math PR to test against. Will check out this PR in the downstream Math repo."),
    string(defaultValue: 'nightly', name: 'stanc3_bin_url',
            description: 'Custom stanc3 binary url')
  ])
])

def runRemainingStages = false
def WIN_CXX = 'g++'

catchError {
  withEnv([
    'MAC_CXX=clang++',
    'LINUX_CXX=clang++-6.0',
    'MPICXX=mpicxx.openmpi',
    'GIT_AUTHOR_NAME=Stan Jenkins',
    'GIT_AUTHOR_EMAIL=mc.stanislaw@gmail.com',
    'GIT_COMMITTER_NAME=Stan Jenkins',
    'GIT_COMMITTER_EMAIL=mc.stanislaw@gmail.com'
  ]) {
    runPod(image: "stanorg/ci:gpu", cpus: 2) {
      runRemainingStages = filesChanged('src/cmdstan', 'src/test', 'lib', 'examples', 'make', 'stan', 'install-tbb.bat', 'makefile', 'runCmdStanTests.py', 'test-all.sh', 'Jenkinsfile')

      stage('clang-format') {
        def dirty = sh returnStatus: true, script: """
          clang-format --version
          git ls-files 'src/*.hpp' 'src/*.cpp' | xargs -n20 -P\$PARALLEL clang-format -i
          git diff --exit-code
        """
        if (dirty) {
          def branch = env.CHANGE_BRANCH ?: env.BRANCH_NAME
          def repo = env.CHANGE_FORK ?: "stan-dev"
          if (!("/" in repo))
            repo += "/cmdstan.git"
          echo "Exiting build because clang-format found changes."
          emailext (
              subject: "[StanJenkins] Autoformattted: Job '${env.JOB_NAME} [${env.BUILD_NUMBER}]'",
              body: """
  Job '${env.JOB_NAME} [${env.BUILD_NUMBER}]' has been autoformatted and the
  changes committed to your branch, if permissions allowed.  Please pull these
  changes before continuing.

  See https://github.com/stan-dev/stan/wiki/Coding-Style-and-Idioms for setting
  up the autoformatter locally.  (Check console output at ${env.BUILD_URL})
  """,
              recipientProviders: [[$class: 'RequesterRecipientProvider']],
              to: env.CHANGE_AUTHOR_EMAIL)
          sh '''
            git add -u src
            git commit -m "[Jenkins] auto-formatting by `clang-format --version`"
          '''
          gitPush(gitScm: scmGit(
              userRemoteConfigs: [[credentialsId: "stan-github", name: 'dest', url: "https://github.com/$repo"]],
              branches: [[name: "refs/heads/$branch"]]),
              targetBranch: branch,
              targetRepo: 'dest')
          echo "Those changes are now found on stan-dev/cmdstan under $repo branch $branch"
          echo "Please 'git pull' before continuing to develop."
          error "clang-format changes"
        }
      }
    }

    if (runRemainingStages) {
      parallel windows: {
        node('windows') {
          stage('Windows interface tests') {
            checkout scmGit(
              branches: scm.branches,
              userRemoteConfigs: scm.userRemoteConfigs,
              extensions: scm.extensions + [cleanBeforeCheckout(),
                submodule(recursiveSubmodules: true, shallow: true, depth: 2)])

            def local = "CXX=${WIN_CXX}\nCXXFLAGS+=-Wp,-D_GLIBCXX_ASSERTIONS\n"
            if (params.stanc3_bin_url != "nightly") {
              local += "STANC3_TEST_BIN_URL=${params.stanc3_bin_url}\n"
            }
            writeFile(file: "make/local", text: local)
            withEnv(["PATH+TBB=${WORKSPACE}\\stan\\lib\\stan_math\\lib\\tbb"]) {
              bat '''
                  SET "PATH=%RTOOLS%;%RTOOLS%\\usr\\bin;%PATH%"
                  SET "PATH=%RTOOLS%\\x86_64-w64-mingw32.static.posix\\bin;%PATH%"
                  SET "PATH=%CONDA%;%PATH%"
                  python runCmdStanTests.py -j%PARALLEL% src/test/interface
              '''
            }
          }
        }
      }
    }
  }
}

emailFailure()
