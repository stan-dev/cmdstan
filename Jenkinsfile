def props = [
  buildDiscarder(logRotator(numToKeepStr: '40', daysToKeepStr: '30')),
  parameters([
    string(defaultValue: '', name: 'stan_pr',
           description: "Stan PR to test against. Will check out this PR in the downstream Stan repo."),
    string(defaultValue: '', name: 'math_pr',
           description: "Math PR to test against. Will check out this PR in the downstream Math repo."),
    string(defaultValue: 'nightly', name: 'stanc3_bin_url',
            description: 'Custom stanc3 binary url'),
    booleanParam(defaultValue: false, name: 'downsteam', description: 'Run downstream tests from stan (was previously downstream_hotfix [master]/downstream_tests [develop])'),
    booleanParam(defaultValue: false, name: 'run_all', description: 'Pretend all files changes'),
    booleanParam(defaultValue: false, name: 'build_tarballs', description: 'Build tarballs as if for a release'),
  ])
]

if (!params.downstream) {
  props <<= disableConcurrentBuilds(abortPrevious: env.BRANCH_NAME != 'develop')
}

properties(props)

def commit
def runRemainingStages = false
def WIN_CXX = 'g++'
def MPI_CXX = 'mpicxx.openmpi'
def MAC_CXX = 'clang++'
def WINSETENV = '''
  SET "PATH=%RTOOLS%\\x86_64-w64-mingw32.static.posix\\bin;%RTOOLS%;%RTOOLS%\\usr\\bin;%CONDA%;%PATH%"
'''

catchError {
  withEnv([
    'GIT_AUTHOR_NAME=Stan Jenkins',
    'GIT_AUTHOR_EMAIL=mc.stanislaw@gmail.com',
    'GIT_COMMITTER_NAME=Stan Jenkins',
    'GIT_COMMITTER_EMAIL=mc.stanislaw@gmail.com'
  ]) {
    runPod(image: "stanorg/ci:gpu", cpus: 2) {
      commit = sh(returnStdout: true, script: "git rev-parse HEAD").trim()
      runRemainingStages = params.downstream || params.run_all || filesChanged('src/cmdstan', 'src/test', 'lib', 'examples', 'make', 'stan', 'install-tbb.bat', 'makefile', 'runCmdStanTests.py', 'test-all.sh', 'Jenkinsfile')

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

    def scmFull = scmGit(
      branches: scm.branches,
      userRemoteConfigs: scm.userRemoteConfigs,
      extensions: scm.extensions + [cleanBeforeCheckout(),
        submodule(recursiveSubmodules: true, shallow: true, depth: 2)])

    def prepTests = { extra ->
      checkout scmFull
      checkoutPR("stan", params.stan_pr)
      checkoutPR("stan/lib/stan_math", params.math_pr)

      def local = "CXXFLAGS+=-Wp,-D_GLIBCXX_ASSERTIONS -Werror -Wno-unused-command-line-argument\n"
      if (params.stanc3_bin_url != "nightly") {
        local += "STANC3_TEST_BIN_URL=${params.stanc3_bin_url}\n"
      }
      writeFile(file: "make/local", text: local + extra)
    }

    if (runRemainingStages) {
      parallel windows: {
        node('windows') {
          stage('Windows interface tests') {
            prepTests("CXX=${WIN_CXX}")
            withEnv(["PATH+TBB=${WORKSPACE}\\stan\\lib\\stan_math\\lib\\tbb"]) {
              bat """$WINSETENV
                  python runCmdStanTests.py -j%PARALLEL% src/test/interface
              """
            }
          }
        }
      }, linux: {
        runPod(image: "stanorg/ci:gpu", checkout: false) {
          stage('Linux interface tests with MPI') {
            prepTests("""CXX=${MPI_CXX}
STAN_MPI=true
CXX_TYPE=gcc""")
            sh "make build-mpi > build-mpi.log 2>&1"
            sh './runCmdStanTests.py -j$PARALLEL src/test/interface'
          }
        }
      }, mac: {
        node('macos') {
          stage('Mac interface tests') {
            prepTests("CXX=${MAC_CXX}")
            sh 'python3 ./runCmdStanTests.py -j$PARALLEL src/test/interface'
          }
        }
      }, upstream: {
        if (params.downstream || env.CHANGE_TARGET) {
          stage('Upstream CmdStan Performance tests') {
            build(
                job: "CCM/Stan/performance-tests-cmdstan/master",
                parameters: [
                    booleanParam(name: 'downstream', value: true),
                    string(name: 'cmdstan_pr', value: env.BRANCH_NAME),
                    string(name: 'stan_pr', value: params.stan_pr),
                    string(name: 'math_pr', value: params.math_pr),
                    string(name: 'stanc3_bin_url', value: params.stanc3_bin_url),
                ],
                wait: true)
          }
        }
      }
      /* TODO recordIssues? */
    }

    if (env.BRANCH_NAME == 'develop') {
      podTemplate(inheritFrom: 'jnlp') {
        node(POD_LABEL) {
          stage('Update performance tests submodule') {
            updateSubmodule('performance-tests-cmdstan', 'master', 'cmdstan', commit)
          }
        }
      }
    }
    if (env.TAG_NAME || params.build_tarballs) {
      runPod(image: "stanorg/ci:gpu", checkout: false) {
        def download_stanc = { args ->
          def platform = args.platform ?: 'linux';
          if (params.stanc3_bin_url != 'nightly') {
            sh "curl -LO '${params.stanc3_bin_url}/bin/${platform}-stanc' --retry 5 --retry-delay 10"
          } else {
            def tagName = env.TAG_NAME ?: 'nightly';
            sh "curl -LO 'https://github.com/stan-dev/stanc3/releases/download/${tagName}/${platform}-stanc' --retry 5 --retry-delay 10"
          }
        }

        stage("Build tarballs") {
          def version = env.TAG_NAME ? env.TAG_NAME.substring(1, env.TAG_NAME.length()) : env.GIT_COMMIT ;

          dir("cmdstan-${version}"){
            checkout scmGit(
              branches: scm.branches,
              userRemoteConfigs: scm.userRemoteConfigs,
              extensions: scm.extensions + [cleanBeforeCheckout(),
                                            submodule(recursiveSubmodules: true, shallow: true, depth: 2)]);
            if (params.stan_pr)
              checkoutPR("stan", params.stan_pr)
            if (params.math_pr)
              checkoutPR("stan/lib/stan_math", params.math_pr)

            sh "mkdir -p bin"
            dir("bin"){
              for (platform in ["windows", "macos", "linux"]) {
                download_stanc(platform: platform)
              }
            }
          }

          sh "tar --exclude-vcs --hard-dereference -chzf cmdstan-${version}.tar.gz cmdstan-${version}/"

          // build the non-x86-linux tarballs
          sh "rm cmdstan-${version}/bin/*-stanc"
          for(arch in ["arm64", "armel", "armhf", "ppc64el", "s390x"]) {
            def platform = "linux-${arch}"
            download_stanc(platform: platform)
            sh """
               mv ${platform}-stanc cmdstan-${version}/bin/linux-stanc
               tar --exclude-vcs --hard-dereference -chzf 'cmdstan-${version}-linux-${arch}.tar.gz' cmdstan-${version}/
               rm cmdstan-${version}/bin/linux-stanc
            """
          }

          archiveArtifacts '.*.tar.gz'

          if (env.TAG_NAME) {
            withCredentials([usernamePassword(usernameVariable: 'GITHUB_USER', passwordVariable: 'GITHUB_TOKEN', credentialsId: 'stan-github')]) {
              sh "gh release upload ${env.TAG_NAME} ./*.tar.gz || gh release create ${env.TAG_NAME} --draft ./*.tar.gz"
            }
          }
        }
      }
    }
  }
}

emailFailure()
