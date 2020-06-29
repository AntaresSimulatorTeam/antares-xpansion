#!/usr/bin/env groovy

configs = [
	[
		node: 'compil-debian9',
		buildTypes: [ 'Debug', 'Release' ],
		testSteps: [ 'cppcheck', 'unittest' , 'memcheck', 'coverage'],
		publish: true
	],
	[
		node: 'compil-win64-vc14',
		buildTypes: [ 'Release' ],
		testSteps: [ 'unittest' ]
	]
]

@Library("utils") _

properties([gitLabConnection('edgitlab')])

stage('checkout') {
	node {
		handleCheckout()
		stash includes: '**', name: 'source'
	}
}

gitlabBuilds(builds: ['build', 'test', 'publish', 'deploy']) {
	gitlabCommitStatus('build') {
		stage('build') {
			nodeStages = [:]

			configs.each { config ->
				nodeStages[config.node] = {
					node(config.node) {
						cleanWs()

						unstash 'source'

						config.buildTypes.each { buildType ->
							withEnv(["CONAN_CMAKE_PROGRAM=${tool '3.14.0 (jenkins)'}/cmake"]) {
								sh """
									conan install \
										--update \
										--settings build_type=${buildType} \
										--install-folder builds/${buildType} \
										.
								"""
							}

							cmakeBuild (
								buildDir: "builds/${buildType}",
								buildType: "${buildType}",
								cleanBuild: false,
								cmakeArgs: """
									-D CMAKE_INSTALL_PREFIX="${WORKSPACE}/install/${buildType}"
									-D antaresXpansion_WITH_COVERAGE=${config.containsKey('testSteps') && config.testSteps.contains('coverage') ? 'ON' : 'OFF'}
									-D antaresXpansion_BUILD_DOCUMENTATION=always
									-D STATIC_RUNTIME=OFF
									-D CMAKE_POSITION_INDEPENDENT_CODE=ON
									-D ORTOOLS_ROOT="${env.OPT_PATH}/or-tools-7.6"
								""",
								generator: "${env.CMakeGenerator}",
								installation: "3.14.0 (jenkins)",
								steps: [[args: "--config ${buildType} --parallel 4 -v", withCmake: true]]
							)
						}
					}
				}
			}

			parallel nodeStages
		}
	}

	gitlabCommitStatus('test') {
		stage('test') {
			nodeStages = [:]

			configs.each { config ->
				if(config.containsKey('testSteps')) {
					nodeStages[config.node] = {
						node(config.node) {
							if(config.testSteps.contains('unittest')) {
								config.buildTypes.each { buildType ->
									cmake (
										arguments: """
											--build builds/${buildType}
											--target run_unit_tests
											--config ${buildType}
										""",
										installation: '3.14.0 (jenkins)'
									)
								}
							}

							if(config.testSteps.contains('cppcheck')) {
								sh 'mkdir -p cppcheck-reports'
								config.buildTypes.each { buildType ->
									sh """
										${env.OPT_PATH}/${env.CPPCHECK_VERSION}/bin/cppcheck \
											--enable=all --inconclusive \
											--suppress=missingIncludeSystem \
											--xml --xml-version=2 \
											--project=builds/${buildType}/compile_commands.json \
											2>cppcheck-reports/cppcheck-${buildType}-results.xml
									"""
								}
							}

							if(config.testSteps.contains('coverage')) {
								// do coverage reports before functest or memcheck to have clearer view of tested source code
								sh 'rm -rf coverage-reports'
								sh 'cp -r builds/Debug/coverage-reports .'
							}

							if(config.testSteps.contains('functest')) {
								// do something
							}

							if(config.testSteps.contains('memcheck')) {
								// unit tests
								runValgrind (
									childSilentAfterFork: true,
									excludePattern: '',
									generateSuppressions: true,
									ignoreExitCode: true,
									includePattern: 'builds/Debug/unit_tests',
									outputDirectory: "${env.VALGRIND_REPORTS_PATH}",
									outputFileEnding: '.xml',
									programOptions: '',
									removeOldReports: false,
									suppressionFiles: '',
									tool: [
										$class: 'ValgrindToolMemcheck',
										leakCheckLevel: 'full',
										showReachable: true,
										trackOrigins: true,
										undefinedValueErrors: true
									],
									traceChildren: true,
									valgrindExecutable: '',
									valgrindOptions: '',
									workingDirectory: "${env.VALGRIND_REPORTS_PATH}"
								)

								// func test
							}

							if(config.testSteps.contains('helgrind')) {
								// do something
							}

							if(config.testSteps.contains('vera')) {
								// do something
							}
						}
					}
				}
			}

			parallel nodeStages
		}
	}

	gitlabCommitStatus('publish') {
		stage('publish') {
			nodeStages = [:]

			configs.each { config ->
				if(config.containsKey('publish')
				&& config.publish) {
					stage('SonarQube analysis') {
						nodeStages[config.node] = {
							node(config.node) {
								def scannerHome = tool 'SonarQube Scanner'
								withSonarQubeEnv {
									sh "${scannerHome}/bin/sonar-scanner"
								}

								timeout(time: 1, unit: 'HOURS') {
									def qg = waitForQualityGate()
								}
							}
						}
					}
				}
			}

			parallel nodeStages
		}
	}

	gitlabCommitStatus('deploy') {
		stage("deploy") {
		}
	}
}

