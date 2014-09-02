# Makefile for CmdStan.
# This makefile relies heavily on the make defaults for
# make 3.81.
##

# The default target of this Makefile is...
help:

## Disable implicit rules.
.SUFFIXES:

##
# Set default values which can be overridden via file make/local
# - CC: The compiler to use. Expecting g++ or clang++.
# - O: Optimization level. Valid values are {0, 1, 2, 3}.
# - AR: archiver (must specify for cross-compiling)
# - OS: {mac, win, linux}. 
##
CC = clang++
O = 0
O_STANC = 0
AR = ar

##
# create absolute filepaths, set search path for source files
# includes current working directory last to compile user models
# in order to use this makefile from elsewhere all paths in
# compiler and shell commands must be absolute paths
# (e.g. make -f ../.../path/to/this/makefile)
##
CMDSTAN_HOME := $(dir $(lastword $(abspath $(MAKEFILE_LIST))))
STANAPI_HOME := $(CMDSTAN_HOME)stan/
VPATH = $(CMDSTAN_HOME)src:$(STANAPI_HOME)src:.

##
# Library locations
##
EIGEN ?= $(STANAPI_HOME)lib/eigen_3.2.0
BOOST ?= $(STANAPI_HOME)lib/boost_1.54.0
GTEST ?= $(STANAPI_HOME)lib/gtest_1.7.0

##
# Set default compiler options.
## 
CFLAGS = -DBOOST_RESULT_OF_USE_TR1 -DBOOST_NO_DECLTYPE -DBOOST_DISABLE_ASSERTS -I $(CMDSTAN_HOME)src -I $(STANAPI_HOME)src -isystem $(EIGEN) -isystem $(BOOST) -Wall -pipe -DEIGEN_NO_DEBUG
CFLAGS_GTEST = -DGTEST_USE_OWN_TR1_TUPLE
LDLIBS = -L$(CMDSTAN_HOME)bin -lstan
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


# OS is set automatically by this script
##
# These includes should update the following variables
# based on the OS:
#   - CFLAGS
#   - CFLAGS_GTEST
#   - EXE
##
-include $(STANAPI_HOME)make/detect_os


##
# Get information about the version of make.
##
-include $(STANAPI_HOME)make/detect_make

##
# Tell make the default way to compile a .o file.
##
%.o : %.cpp
	$(COMPILE.c) -O$O $(OUTPUT_OPTION) $<

%$(EXE) : %.cpp %.stan $(CMDSTAN_HOME)bin/libstan.a 
	@echo ''
	@echo '--- Linking C++ model ---'
	$(LINK.c) -O$O $(OUTPUT_OPTION) $(CMDSTAN_MAIN) -include $< $(LDLIBS)


##
# Tell make the default way to compile a .o file.
##
$(CMDSTAN_HOME)bin/%.o : %.cpp
	@mkdir -p $(dir $@)
	$(COMPILE.c) -O$O $(OUTPUT_OPTION) $<

### not needed if using VPATH
## Tell make the default way to compile a .o file.
###
#bin/stan/%.o : %.cpp
#	@mkdir -p $(dir $@)
#	$(COMPILE.c) -O$O $(OUTPUT_OPTION) $<


##
# Rule for generating dependencies.
# Applies to all *.cpp files in src.
# Test cpp files are handled slightly differently.
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
	@echo 'CMDSTAN_HOME: ' $(CMDSTAN_HOME)
	@echo 'STANAPI_HOME: ' $(STANAPI_HOME)
	@echo 'VPATH: ' $(VPATH)
	@echo '--------------------------------------------------------------------------------'
	@echo 'Stan makefile:'
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
	@echo 'Build a Stan model:'
	@echo '  Given a Stan model at foo/bar.stan, the make target is:'
	@echo '  - foo/bar$(EXE)'
	@echo ''
	@echo '  This target will:'
	@echo '  1. Build the Stan compiler: bin/stanc$(EXE).'
	@echo '  2. Use the Stan compiler to generate C++ code, foo/bar.cpp.'
	@echo '  3. Compile the C++ code using $(CC) to generate foo/bar$(EXE)'
	@echo ''
	@echo '  Example - Sample from a normal: stan/example-models/basic_distributions/normal.stan'
	@echo '    1. Build the model:'
	@echo '       make stan/example-models/basic_distributions/normal$(EXE)'
	@echo '    2. Run the model:'
	@echo '       stan'$(PATH_SEPARATOR)'example-models'$(PATH_SEPARATOR)'basic_distributions'$(PATH_SEPARATOR)'normal$(EXE) sample'
	@echo '    3. Look at the samples:'
	@echo '       bin'$(PATH_SEPARATOR)'print$(EXE) output.csv'
	@echo ''
	@echo 'Dev-relevant targets:'
	@echo '  Stan management targets:'
	@echo '  - stan-update    : Initializes and updates the Stan repository'
	@echo '  - stan-update/*  : Updates the Stan repository to the specified'
	@echo '                     branch or commit hash.'
	@echo '  - stan-revert    : Reverts changes made to Stan library back to'
	@echo '                     what is in the repository.'
	@echo '  Test targets:'
	@echo '  - src/test/interface: Runs tests on CmdStan interface.'
	@echo '  - src/test/models   : Runs model tests in CmdStan'
	@echo ''
	@echo 'Model related:'
	@echo '- bin/stanc$(EXE): Build the Stan compiler.'
	@echo '- bin/print$(EXE): Build the print utility.'
	@echo '- bin/libstan.a  : Build the Stan static library (used in linking models).'
	@echo '- bin/libstanc.a : Build the Stan compiler static library (used in linking'
	@echo '                   bin/stanc$(EXE))'
	@echo '- *$(EXE)        : If a Stan model exists at *.stan, this target will build'
	@echo '                   the Stan model as an executable.'
	@echo ''
	@echo 'Documentation:'
	@echo ' - manual:          Build the Stan manual and the CmdStan user guide.'
	@echo '--------------------------------------------------------------------------------'
-include $(CMDSTAN_HOME)make/libstan  # libstan.a
-include $(CMDSTAN_HOME)make/models   # models
-include $(CMDSTAN_HOME)make/tests
-include $(CMDSTAN_HOME)make/command  # bin/stanc, bin/print
-include $(STANAPI_HOME)make/manual

.PHONY: build
build: $(CMDSTAN_HOME)bin/stanc$(EXE) $(CMDSTAN_HOME)bin/print$(EXE)
	@echo ''
	@echo 'CMDSTAN_HOME: ' $(CMDSTAN_HOME)
	@echo 'STANAPI_HOME: ' $(STANAPI_HOME)
	@echo 'VPATH: ' $(VPATH)
	@echo ''
	@echo '--- Stan tools built ---'

##
# Clean up.
##
.PHONY: clean clean-manual clean-all

clean: clean-manual
	$(RM) -r $(CMDSTAN_HOME)test
	$(RM) $(wildcard $(patsubst %.stan,%.cpp,$(TEST_MODELS)))
	$(RM) $(wildcard $(patsubst %.stan,%$(EXE),$(TEST_MODELS)))

clean-manual:
	cd $(CMDSTAN_HOME)src/docs/cmdstan-guide; $(RM) *.brf *.aux *.bbl *.blg *.log *.toc *.pdf *.out *.idx *.ilg *.ind *.cb *.cb2 *.upa


clean-all: clean
	$(RM) -r $(CMDSTAN_HOME)bin

##
# Submodule related tasks
##

.PHONY: stan-update
stan-update :
	git submodule init
	git submodule update --recursive

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
manual: $(CMDSTAN_HOME)src/docs/cmdstan-guide/cmdstan-guide.pdf
