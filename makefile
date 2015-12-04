# Makefile for CmdStan.
# This makefile relies heavily on the make defaults for
# make 3.81.
##

# The default target of this Makefile is...
help:

## Disable implicit rules.
.SUFFIXES:

##
# Users should only need to set these three variables for use.
# - CC: The compiler to use. Expecting g++ or clang++.
# - O: Optimization level. Valid values are {0, 1, 2, 3}.
# - AR: archiver (must specify for cross-compiling)
# - OS: {mac, win, linux}. 
##
CC = g++
O = 3
O_STANC = 0
AR = ar

##
# Library locations
##
STAN ?= stan_2.9.0/
MATH ?= $(STAN)lib/stan_math_2.9.0/
-include $(MATH)make/libraries

##
# Set default compiler options.
## 
CFLAGS = -DBOOST_RESULT_OF_USE_TR1 -DBOOST_NO_DECLTYPE -DBOOST_DISABLE_ASSERTS -I src -I $(STAN)src -isystem $(MATH) -isystem $(EIGEN) -isystem $(BOOST) -Wall -pipe -DEIGEN_NO_DEBUG
CFLAGS_GTEST = -DGTEST_USE_OWN_TR1_TUPLE
LDLIBS =
LDLIBS_STANC = -Lbin -lstanc
EXE = 
PATH_SEPARATOR = /
CMDSTAN_VERSION := 2.9.0

##
# Get information about the compiler used.
# - CC_TYPE: {g++, clang++, mingw32-g++, other}
# - CC_MAJOR: major version of CC
# - CC_MINOR: minor version of CC
##
-include $(STAN)make/detect_cc

# OS is set automatically by this script
##
# These includes should update the following variables
# based on the OS:
#   - CFLAGS
#   - CFLAGS_GTEST
#   - EXE
##
-include $(STAN)make/detect_os

##
# Get information about the version of make.
##
-include $(STAN)make/detect_make

##
# Tell make the default way to compile a .o file.
##
%.o : %.cpp
	$(COMPILE.c) -O$O $(OUTPUT_OPTION) $<

%$(EXE) : %.hpp %.stan 
	@echo ''
	@echo '--- Linking C++ model ---'
	$(LINK.c) -O$O $(OUTPUT_OPTION) $(CMDSTAN_MAIN) -include $< $(LDLIBS)

##
# Tell make the default way to compile a .o file.
##
bin/%.o : src/%.cpp
	@mkdir -p $(dir $@)
	$(COMPILE.c) -O$O $(OUTPUT_OPTION) $<

##
# Tell make the default way to compile a .o file.
##
bin/stan/%.o : $(STAN)src/stan/%.cpp
	@mkdir -p $(dir $@)
	$(COMPILE.c) -O$O $(OUTPUT_OPTION) $<


##
# Rule for generating dependencies.
# Applies to all *.cpp files in src.
# Test cpp files are handled slightly differently.
##
bin/%.d : src/%.cpp
	@if test -d $(dir $@); \
	then \
	(set -e; \
	rm -f $@; \
	$(CC) $(CFLAGS) -O$O $(TARGET_ARCH) -MM $< > $@.$$$$; \
	sed -e 's,\($(notdir $*)\)\.o[ :]*,$(dir $@)\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$);\
	fi

%.d : %.cpp
	@if test -d $(dir $@); \
	then \
	(set -e; \
	rm -f $@; \
	$(CC) $(CFLAGS) -O$O $(TARGET_ARCH) -MM $< > $@.$$$$; \
	sed -e 's,\($(notdir $*)\)\.o[ :]*,$(dir $@)\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$);\
	fi

.PHONY: help
help:	
	@echo '--------------------------------------------------------------------------------'
	@echo 'CmdStan v$(CMDSTAN_VERSION) help'
	@echo ''
	@echo '  Build CmdStan utilities:'
	@echo '    > make build'
	@echo ''
	@echo '    This target will:'
	@echo '    1. Build the Stan compiler bin/stanc$(EXE).'
	@echo '    2. Build the print utility bin/print$(EXE) (deprecated; will be removed in v3.0)'
	@echo '    3. Build the stansummary utility bin/stansummary$(EXE)'
	@echo ''
	@echo '    Note: to build using multiple cores, use the -j option to make. '
	@echo '    For 4 cores:'
	@echo '    > make build -j4'
	@echo ''
	@echo ''
	@echo '  Build a Stan program:'
	@echo ''
	@echo '    Given a Stan program at foo/bar.stan, build an executable by typing:'
	@echo '    > make foo/bar$(EXE)'
	@echo ''
	@echo '    This target will:'
	@echo '    1. Build the Stan compiler and the print utility if not built.'
	@echo '    2. Use the Stan compiler to generate C++ code, foo/bar.hpp.'
	@echo '    3. Compile the C++ code using $(CC) $(CC_MAJOR).$(CC_MINOR) to generate foo/bar$(EXE)'
	@echo ''
	@echo ''
	@echo '  Example - bernoulli model: examples/bernoulli/bernoulli.stan'
	@echo ''
	@echo '    1. Build the model:'
	@echo '       > make examples/bernoulli/bernoulli$(EXE)'
	@echo '    2. Run the model:'
	@echo '       > examples'$(PATH_SEPARATOR)'bernoulli'$(PATH_SEPARATOR)'bernoulli$(EXE) sample data file=examples/bernoulli/bernoulli.data.R'
	@echo '    3. Look at the samples:'
	@echo '       > bin'$(PATH_SEPARATOR)'stansummary$(EXE) output.csv'
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
	@echo '  Current configuration:'
	@echo '  - OS (Operating System):   ' $(OS)
	@echo '  - CC (Compiler):           ' $(CC)
	@echo '  - Compiler version:        ' $(CC_MAJOR).$(CC_MINOR)
	@echo '  - O (Optimization Level):  ' $(O)
	@echo '  - O_STANC (Opt for stanc): ' $(O_STANC)
ifdef TEMPLATE_DEPTH
	@echo '  - TEMPLATE_DEPTH:          ' $(TEMPLATE_DEPTH)
endif
	@echo '  Library configuration:'
	@echo '  - EIGEN                    ' $(EIGEN)
	@echo '  - BOOST                    ' $(BOOST)
	@echo '  - GTEST                    ' $(GTEST)
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
	@echo '- bin/stanc$(EXE): Build the Stan compiler.'
	@echo '- bin/print$(EXE): Build the print utility. (deprecated)'
	@echo '- bin/stansummary(EXE): Build the print utility.'
	@echo '- bin/libstanc.a : Build the Stan compiler static library (used in linking'
	@echo '                   bin/stanc$(EXE))'
	@echo '- *$(EXE)        : If a Stan model exists at *.stan, this target will build'
	@echo '                   the Stan model as an executable.'
	@echo ''
	@echo 'Documentation:'
	@echo ' - manual:          Build the Stan manual and the CmdStan user guide.'
	@echo '--------------------------------------------------------------------------------'

-include $(HOME)/.config/cmdstan/make.local    # define local variables
-include make/local    # for local variables
-include make/libstan  # libstan.a
-include make/models   # models
-include make/tests
-include make/command  # bin/stanc, bin/stansummary, bin/print
-include $(STAN)make/manual

.PHONY: build
build: bin/stanc$(EXE) bin/stansummary$(EXE) bin/print$(EXE)
	@echo ''
	@echo '--- CmdStan v$(CMDSTAN_VERSION) built ---'

##
# Clean up.
##
.PHONY: clean clean-manual clean-all

clean: clean-manual
	$(RM) -r test
	$(RM) $(wildcard $(patsubst %.stan,%.hpp,$(TEST_MODELS)))
	$(RM) $(wildcard $(patsubst %.stan,%$(EXE),$(TEST_MODELS)))

clean-manual:
	cd src/docs/cmdstan-guide; $(RM) *.brf *.aux *.bbl *.blg *.log *.toc *.pdf *.out *.idx *.ilg *.ind *.cb *.cb2 *.upa


clean-all: clean
	$(RM) -r bin

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

