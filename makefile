# Makefile for CmdStan.
##

# Default target.
help:

## Disable implicit rules.
.SUFFIXES:

##
# Create absolute filepaths, set search path for source files
# so that this makefile can be referenced from elsewhere,
# e.g., make -f ../.../path/to/this/makefile
##
CMDSTAN_HOME := $(dir $(lastword $(abspath $(MAKEFILE_LIST))))
STANAPI_HOME := $(CMDSTAN_HOME)stan/
VPATH = $(CMDSTAN_HOME)src:$(STANAPI_HOME)src:.

##
# Set default values which can be overridden via file make/local
# - CC: The compiler to use. Expecting g++ or clang++.
# - O: Optimization level for models. Valid values are {0, 1, 2, 3}.
# - O_STANC: Optimization level for stan binaries. Valid values are {0, 1, 2, 3}.
# - AR: archiver (must specify for cross-compiling)
##
CC = g++
O = 3
O_STANC = 0
AR = ar
C++11 = false

##
# Library locations
##
STANAPI_HOME ?= $(CMDSTAN_HOME)stan/
EIGEN ?= $(STANAPI_HOME)lib/eigen_3.2.4
BOOST ?= $(STANAPI_HOME)lib/boost_1.58.0
GTEST ?= $(STANAPI_HOME)lib/gtest_1.7.0
MATH  ?= $(STANAPI_HOME)lib/stan_math_2.6.3

##
# Set default compiler options.
## 
CFLAGS = -DBOOST_RESULT_OF_USE_TR1 -DBOOST_NO_DECLTYPE -DBOOST_DISABLE_ASSERTS -I $(CMDSTAN_HOME)src -I $(STANAPI_HOME)src -isystem $(MATH) -isystem $(EIGEN) -isystem $(BOOST) -Wall -pipe -DEIGEN_NO_DEBUG
CFLAGS_GTEST = -DGTEST_USE_OWN_TR1_TUPLE
LDLIBS = 
LDLIBS_STANC = -L$(CMDSTAN_HOME)bin -lstanc
EXE = 
PATH_SEPARATOR = /


##
# Get information about the compiler used.
# - CC_TYPE: {g++, clang++, mingw32-g++, other}
# - CC_MAJOR: major version of CC
# - CC_MINOR: minor version of CC
##
-include $(STANAPI_HOME)make/detect_cc

## 
# Detect OS_TYPE
-include $(STANAPI_HOME)make/detect_os

##
# Update the following variables based on the OS_TYPE:
#   - CFLAGS
#   - EXE
##
-include $(STANAPI_HOME)make/os_$(OS_TYPE)

##
# Rules to compile .stan models
##
%.o : %.cpp
	$(COMPILE.c) -O$O $(OUTPUT_OPTION) $<

%$(EXE) : %.hpp %.stan 
	@echo ''
	@echo '--- Linking C++ model ---'
	$(LINK.c) -O$O $(OUTPUT_OPTION) $(CMDSTAN_MAIN) -include $< $(LDLIBS)

##
# Rule to compile stan binaries.
##
$(CMDSTAN_HOME)bin/%.o : %.cpp
	@mkdir -p $(dir $@)
	$(COMPILE.c) -O$O $(OUTPUT_OPTION) $<



##
# Rule for generating dependencies for *.cpp files.
##
bin/%.d : %.cpp
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
	@echo 'CmdStan makefile'
	@echo ' Configuration:'
	@echo '  - OS_TYPE (Operating System):   ' $(OS_TYPE)
	@echo '  - CC (Compiler):           ' $(CC)
	@echo '  - Compiler version:        ' $(CC_MAJOR).$(CC_MINOR)
	@echo '  - O (Optimization Level):  ' $(O)
	@echo '  - O_STANC (Opt for stanc): ' $(O_STANC)
	@echo '  - EXE (suffix for executable): ' $(EXE)
ifdef TEMPLATE_DEPTH
	@echo '  - TEMPLATE_DEPTH:          ' $(TEMPLATE_DEPTH)
endif
	@echo '  Library configuration:'
	@echo '  - EIGEN                    ' $(EIGEN)
	@echo '  - BOOST                    ' $(BOOST)
	@echo ''
	@echo 'Build CmdStan utilities:'
	@echo '  - build'
	@echo ''
	@echo '  This target will:'
	@echo '  1. Build the Stan Compiler bin/stanc$(EXE).'
	@echo '  2. Build the print utility bin/print$(EXE)'
	@echo ''
	@echo '  Before building CmdStan utilities the first time you need'
	@echo '  to initialize the Stan repository with:'
	@echo '     make stan-update'
	@echo ''
	@echo 'Build a Stan model:'
	@echo '  Given a Stan model at foo/bar.stan, the make target is:'
	@echo '  - foo/bar$(EXE)'
	@echo ''
	@echo '  This target will:'
	@echo '  1. Build the Stan compiler: bin/stanc$(EXE).'
	@echo '  2. Use the Stan compiler to generate C++ code, foo/bar.hpp.'
	@echo '  3. Compile the C++ code using $(CC) to generate foo/bar$(EXE)'
	@echo ''
	@echo '  Example - Sample from a normal: example-models/basic_distributions/normal.stan'
	@echo '    1. Build the model:'
	@echo '       make example-models/basic_distributions/normal$(EXE)'
	@echo '    2. Run the model:'
	@echo '       example-models'$(PATH_SEPARATOR)'basic_distributions'$(PATH_SEPARATOR)'normal$(EXE) sample'
	@echo '    3. Look at the samples:'
	@echo '       bin'$(PATH_SEPARATOR)'print$(EXE) output.csv'
	@echo ''
	@echo ' For developer targets:'
	@echo ' - help-dev'
	@echo '--------------------------------------------------------------------------------'

.PHONY: help-dev
help-dev:
	@echo '--------------------------------------------------------------------------------'
	@echo 'CmdStan developer targets - must be in CmdStan home directory to run these.'
	@echo '  Stan management targets:'
	@echo '  - stan-update    : Initializes and updates the Stan repository'
	@echo '  - stan-update/*  : Updates the Stan repository to the specified'
	@echo '                     branch or commit hash.'
	@echo '  - stan-revert    : Reverts changes made to Stan library back to'
	@echo '                     what is in the repository.'
	@echo '  Test targets: run these via script runCmdStan.py'
	@echo '  - src/test/interface: Runs tests on CmdStan interface.'
	@echo ''
	@echo 'Model related:'
	@echo '- bin/stanc$(EXE): Build the Stan compiler.'
	@echo '- bin/print$(EXE): Build the print utility.'
	@echo '- bin/libstanc.a : Build the Stan compiler static library (used in linking'
	@echo '                   bin/stanc$(EXE))'
	@echo ''
	@echo 'Documentation:'
	@echo ' - manual:          Build the Stan manual and the CmdStan user guide.'
	@echo '--------------------------------------------------------------------------------'

# common tasks
-include $(CMDSTAN_HOME)make/local    # for local variables
-include $(CMDSTAN_HOME)make/libstan  # libstanc.a
-include $(CMDSTAN_HOME)make/models   # models
-include $(CMDSTAN_HOME)make/command  # bin/stanc, bin/print

# developer tasks
-include $(CMDSTAN_HOME)make/tests
-include $(STANAPI_HOME)make/manual

.PHONY: build
build: $(CMDSTAN_HOME)bin/stanc$(EXE) $(CMDSTAN_HOME)bin/print$(EXE)
	@echo ''
	@echo '--- CmdStan built ---'

##
# Clean up.  Removes Stan compiler and libraries.
##
.PHONY: clean clean-manual clean-all

clean: clean-manual
	$(RM) -r $(CMDSTAN_HOME)test
	$(RM) $(wildcard $(patsubst %.stan,%.hpp,$(TEST_MODELS)))
	$(RM) $(wildcard $(patsubst %.stan,%$(EXE),$(TEST_MODELS)))

clean-manual:
	cd $(CMDSTAN_HOME)src/docs/cmdstan-guide; $(RM) *.brf *.aux *.bbl *.blg *.log *.toc *.pdf *.out *.idx *.ilg *.ind *.cb *.cb2 *.upa


clean-all: clean
	$(RM) -r $(CMDSTAN_HOME)bin


##
# CmdStan manual.
##
.PHONY: $(CMDSTAN_HOME)src/docs/cmdstan-guide/cmdstan-guide.tex
manual: $(CMDSTAN_HOME)src/docs/cmdstan-guide/cmdstan-guide.pdf


##
# Submodule related tasks
##

.PHONY: stan-update
stan-update :
	git submodule init
	git submodule update --recursive

stan-update/%: stan-update
	cd $(STANAPI_HOME) && git fetch --all && git checkout $* && git pull

stan-pr/%: stan-update
	cd $(STANAPI_HOME) && git reset --hard origin/develop && git checkout $* && git checkout develop && git merge $* --ff --no-edit --strategy=ours

.PHONY: stan-revert
stan-revert:
	git submodule update --init --recursive


