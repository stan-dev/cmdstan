##
# CmdStan users: if you need to customize make options,
#   you should add variables to a new file called
#   make/local (no file extension)
#
# A typical option might be:
#   CXX = clang++
#
# Users should only need to set these variables:
# - CXX: The compiler to use. Expecting g++ or clang++.
# - O: Optimization level. Valid values are {s, 0, 1, 2, 3}.
#      Default is 3.
# - O_STANC: Optimization level for compiling stanc.
#      Valid values are {s, 0, 1, 2, 3}. Default is 0
# - STANCFLAGS: Extra options for calling stanc
##

# The default target of this Makefile is...
help:

-include $(HOME)/.config/stan/make.local  # user-defined variables
-include make/local                       # user-defined variables

STAN ?= stan/
MATH ?= $(STAN)lib/stan_math/
RAPIDJSON ?= lib/rapidjson_1.1.0/
ifeq ($(OS),Windows_NT)
  O_STANC ?= 3
endif
O_STANC ?= 0
INC_FIRST ?= -I src -I $(STAN)src -I $(RAPIDJSON)
USER_HEADER ?= $(dir $<)user_header.hpp


-include $(MATH)make/compiler_flags
-include $(MATH)make/dependencies
-include $(MATH)make/libraries
include make/stanc
include make/program
include make/tests
include make/command

ifneq ($(filter-out clean clean-% print-% help help-% manual stan-update/% stan-update stan-pr/%,$(MAKECMDGOALS)),)
-include $(patsubst %.cpp,%.d,$(STANC_TEMPLATE_INSTANTIATION_CPP))
-include src/cmdstan/stanc.d
endif

CMDSTAN_VERSION := 2.22.1

.PHONY: help
help:
	@echo '--------------------------------------------------------------------------------'
	@echo 'CmdStan v$(CMDSTAN_VERSION) help'
	@echo ''
	@echo '  Build CmdStan utilities:'
ifeq ($(OS),Windows_NT)
	@echo '    > mingw32-make build'
else
	@echo '    > make build'
endif
	@echo ''
	@echo '    This target will:'
	@echo '    1. Install the Stan compiler bin/stanc$(EXE) from stanc3 binaries and build bin/stanc2$(EXE).'
	@echo '    2. Build the print utility bin/print$(EXE) (deprecated; will be removed in v3.0)'
	@echo '    3. Build the stansummary utility bin/stansummary$(EXE)'
	@echo '    4. Build the diagnose utility bin/diagnose$(EXE)'
	@echo '    5. Build all libraries and object files compile and link an executable Stan program'
	@echo ''
	@echo '    Note: to build using multiple cores, use the -j option to make, e.g., '
	@echo '    for 4 cores:'
ifeq ($(OS),Windows_NT)
	@echo '    > mingw32-make build -j4'
else
	@echo '    > make build -j4'
endif
	@echo ''
ifeq ($(OS),Windows_NT)
	@echo '    On Windows it is recommended to include with the PATH environment'
	@echo '    variable the directory of the Intel TBB library.'
	@echo '    This can be setup permanently for the user with'
	@echo '    > mingw32-make install-tbb'
endif
	@echo ''
	@echo '  Build a Stan program:'
	@echo ''
	@echo '    Given a Stan program at foo/bar.stan, build an executable by typing:'
	@echo '    > make foo/bar$(EXE)'
	@echo ''
	@echo '    This target will:'
	@echo '    1. Install the Stan compiler (bin/stanc or bin/stanc2), as needed.'
	@echo '    2. Use the Stan compiler to generate C++ code, foo/bar.hpp.'
	@echo '    3. Compile the C++ code using $(CC) $(CC_MAJOR).$(CC_MINOR) to generate foo/bar$(EXE)'
	@echo ''
	@echo '  Additional make options:'
	@echo '    STANCFLAGS: defaults to "". These are extra options passed to bin/stanc$(EXE)'
	@echo '      when generating C++ code. If you want to allow undefined functions in the'
	@echo '      Stan program, either add this to make/local or the command line:'
	@echo '          STANCFLAGS = --allow_undefined'
	@echo '    USER_HEADER: when STANCFLAGS has --allow_undefined, this is the name of the'
	@echo '      header file that is included. This defaults to "user_header.hpp" in the'
	@echo '      directory of the Stan program.'
	@echo '    STANC2: When set, use bin/stanc2 to generate C++ code.'
	@echo ''
	@echo ''
	@echo '  Example - bernoulli model: examples/bernoulli/bernoulli.stan'
	@echo ''
	@echo '    1. Build the model:'
	@echo '       > make examples/bernoulli/bernoulli$(EXE)'
	@echo '    2. Run the model:'
ifeq ($(OS),Windows_NT)
	@echo '       > examples\bernoulli\bernoulli$(EXE) sample data file=examples/bernoulli/bernoulli.data.R'
else
	@echo '       > examples/bernoulli/bernoulli$(EXE) sample data file=examples/bernoulli/bernoulli.data.R'
endif
	@echo '    3. Look at the samples:'
ifeq ($(OS),Windows_NT)
	@echo '       > bin\stansummary$(EXE) output.csv'
else
	@echo '       > bin/stansummary$(EXE) output.csv'
endif
	@echo ''
	@echo ''
	@echo '  Clean CmdStan:'
	@echo ''
	@echo '    Remove the built CmdStan tools:'
	@echo '    > make clean-all'
	@echo ''
	@echo '--------------------------------------------------------------------------------'

.PHONY: help-dev
help-dev:
	@echo '--------------------------------------------------------------------------------'
	@echo 'CmdStan help for developers:'
	@$(MAKE) print-compiler-flags
	@echo '  - O_STANC (Opt for stanc):    ' $(O_STANC)
	@echo ''
	@echo '  If this copy of CmdStan has been cloned using git,'
	@echo '  before building CmdStan utilities the first time you need'
	@echo '  to initialize the Stan repository with:'
	@echo '     make stan-update'
	@echo ''
	@echo ''
	@echo 'Developer relevant targets:'
	@echo '  Stan management targets:'
	@echo '  - stan-update    : Initializes and updates the Stan repository'
	@echo '  - stan-update/*  : Updates the Stan repository to the specified'
	@echo '                     branch or commit hash.'
	@echo '  - stan-revert    : Reverts changes made to Stan library back to'
	@echo '                     what is in the repository.'
	@echo ''
	@echo 'Model related:'
	@echo '- bin/stanc$(EXE): Download the Stan compiler binary.'
	@echo '- bin/print$(EXE): Build the print utility. (deprecated)'
	@echo '- bin/stansummary$(EXE): Build the print utility.'
	@echo '- bin/diagnostic$(EXE): Build the diagnostic utility.'
	@echo ''
	@echo '- *$(EXE)        : If a Stan model exists at *.stan, this target will build'
	@echo '                   the Stan model as an executable.'
	@echo '- compile_info   : prints compiler flags for compiling a CmdStan executable.'
	@echo ''
	@echo 'Documentation:'
	@echo ' - manual:          Build the Stan manual and the CmdStan user guide.'
	@echo '--------------------------------------------------------------------------------'

.PHONY: build-mpi
build-mpi: $(MPI_TARGETS)
	@echo ''
	@echo '--- boost mpi bindings built ---'

ifeq ($(CMDSTAN_SUBMODULES),1)
.PHONY: build
build: bin/stanc$(EXE) bin/stanc2$(EXE) bin/stansummary$(EXE) bin/print$(EXE) bin/diagnose$(EXE) $(LIBSUNDIALS) $(MPI_TARGETS) $(TBB_TARGETS) $(CMDSTAN_MAIN_O)
	@echo ''
ifeq ($(OS),Windows_NT)
		@echo 'NOTE: Please add $(TBB_BIN_ABSOLUTE_PATH) to your PATH variable.'
		@echo 'You may call'
		@echo ''
		@echo 'mingw32-make install-tbb'
		@echo ''
		@echo 'to automatically update your user configuration.'
endif
	@echo '--- CmdStan v$(CMDSTAN_VERSION) built ---'
else
.PHONY: build
build:
	@echo 'ERROR: Missing Stan submodules.'
	@echo 'Please run the following commands to fix:'
	@echo ''
	@echo 'git submodule init'
	@echo 'git submodule update --recursive'
	@echo ''
	@echo 'And try building again'
	@exit 1
endif

ifeq ($(CXX_TYPE),clang)
build: $(STAN)src/stan/model/model_header.hpp.gch
endif

.PHONY: install-tbb
install-tbb: $(TBB_TARGETS)
ifeq ($(OS),Windows_NT)
	$(shell echo "cmd.exe /C install-tbb.bat")
endif

##
# Clean up.
##
.PHONY: clean clean-deps clean-manual clean-all clean-program

clean: clean-manual
	$(RM) -r test
	$(RM) $(wildcard $(patsubst %.stan,%.d,$(TEST_MODELS)))
	$(RM) $(wildcard $(patsubst %.stan,%.hpp,$(TEST_MODELS)))
	$(RM) $(wildcard $(patsubst %.stan,%.o,$(TEST_MODELS)))
	$(RM) $(wildcard $(patsubst %.stan,%$(EXE),$(TEST_MODELS)))

clean-deps:
	@echo '  removing dependency files'
	$(shell find src $(STAN)src/stan $(MATH)stan -type f -name '*.d' -exec rm {} +)
	$(shell find src $(STAN)src/stan $(MATH)stan -type f -name '*.d.*' -exec rm {} +)
	$(shell find src $(STAN)src/stan $(MATH)stan -type f -name '*.dSYM' -exec rm {} +)

clean-manual:
	$(RM) -r doc
	cd src/docs/cmdstan-guide; $(RM) *.brf *.aux *.bbl *.blg *.log *.toc *.pdf *.out *.idx *.ilg *.ind *.cb *.cb2 *.upa

clean-all: clean clean-deps clean-libraries clean-manual
	$(RM) bin/stanc$(EXE) bin/stanc2$(EXE) bin/stansummary$(EXE) bin/print$(EXE) bin/diagnose$(EXE)
	$(RM) -r $(CMDSTAN_MAIN_O) bin/cmdstan
	$(RM) $(wildcard $(STAN)src/stan/model/model_header.hpp.gch)
	$(RM) examples/bernoulli/bernoulli$(EXE) examples/bernoulli/bernoulli.o examples/bernoulli/bernoulli.d examples/bernoulli/bernoulli.hpp


clean-program:
ifndef STANPROG
	$(error STANPROG not set)
endif
	$(RM) "$(wildcard $(patsubst %.stan,%.d,$(basename ${STANPROG}).stan))"
	$(RM) "$(wildcard $(patsubst %.stan,%.hpp,$(basename ${STANPROG}).stan))"
	$(RM) "$(wildcard $(patsubst %.stan,%.o,$(basename ${STANPROG}).stan))"
	$(RM) "$(wildcard $(patsubst %.stan,%$(EXE),$(basename ${STANPROG}).stan))"

##
# Submodule related tasks
##

.PHONY: stan-update
stan-update :
	git submodule update --init --recursive

stan-update/%: stan-update
	cd stan && git fetch --all && git checkout $* && git pull

stan-pr/%: stan-update
	cd stan && git reset --hard origin/develop && git checkout $* && git checkout develop && git merge $* --ff --no-edit --strategy=ours

.PHONY: stan-revert
stan-revert:
	git submodule update --init --recursive


##
# Manual related
##

.PHONY: src/docs/cmdstan-guide/cmdstan-guide.tex
manual: src/docs/cmdstan-guide/cmdstan-guide.pdf
	mkdir -p doc
	mv -f src/docs/cmdstan-guide/cmdstan-guide.pdf doc/cmdstan-guide-$(CMDSTAN_VERSION).pdf

%.pdf: %.tex
	cd $(dir $@); latexmk -pdf -pdflatex="pdflatex -file-line-error" -use-make $(notdir $^)


.PHONY: compile_info
compile_info:
	@echo '$(LINK.cpp) $(CXXFLAGS_PROGRAM) $(CMDSTAN_MAIN_O) $(LDLIBS) $(LIBSUNDIALS) $(MPI_TARGETS) $(TBB_TARGETS)'

##
# Debug target that allows you to print a variable
##
.PHONY: print-%
print-%  : ; @echo $* = $($*)
