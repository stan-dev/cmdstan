# Makefile for CmdStan.
# User-facing rules for binaries and Stan models.
# Rules for developer tasks are in makefile.developer.
##

# Default target.
help:

## Disable implicit rules.
.SUFFIXES:

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
EIGEN ?= $(STANAPI_HOME)lib/eigen_3.2.0
BOOST ?= $(STANAPI_HOME)lib/boost_1.54.0

##
# Set default compiler options.
## 
CFLAGS = -DBOOST_RESULT_OF_USE_TR1 -DBOOST_NO_DECLTYPE -DBOOST_DISABLE_ASSERTS -I $(CMDSTAN_HOME)src -I $(STANAPI_HOME)src -isystem $(EIGEN) -isystem $(BOOST) -Wall -pipe -DEIGEN_NO_DEBUG
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

## 
# Detect OS_TYPE
-include $(STANAPI_HOME)make/detect_os

##
# Update the following variables based on the OS_TYPE:
#   - CFLAGS
#   - EXE
##
-include $(CMDSTAN_HOME)make/os_$(OS_TYPE)

##
# Rules to compile .stan models
##
%.o : %.cpp
	$(COMPILE.c) -O$O $(OUTPUT_OPTION) $<

%$(EXE) : %.cpp %.stan $(CMDSTAN_HOME)bin/libstan.a 
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
ifdef TEMPLATE_DEPTH
	@echo '  - TEMPLATE_DEPTH:          ' $(TEMPLATE_DEPTH)
endif
	@echo '  Library configuration:'
	@echo '  - EIGEN                    ' $(EIGEN)
	@echo '  - BOOST                    ' $(BOOST)
	@echo ''
	@echo ' Targets:'
	@echo ''
	@echo ' Build the Stan biniaries bin/stanc$(EXE) and bin/print$(EXE):'
	@echo '  - build'
	@echo ''
	@echo ' Build a Stan model:'
	@echo '  Given a Stan model at foo/bar.stan, the make target is:'
	@echo '  - foo/bar$(EXE)'
	@echo ''
	@echo '  make will:'
	@echo '  1. If necessary, build the Stan compiler: bin/stanc$(EXE)'
	@echo '     Called only if no bin/stanc$(EXE) found.  See target "build"'
	@echo '  2. Use the Stan compiler to generate C++ code, foo/bar.cpp'
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
	@echo '--------------------------------------------------------------------------------'
-include $(CMDSTAN_HOME)make/local    # for local variables
-include $(CMDSTAN_HOME)make/libstan  # libstan.a
-include $(CMDSTAN_HOME)make/models   # models
-include $(CMDSTAN_HOME)make/command  # bin/stanc, bin/print

.PHONY: build
build: $(CMDSTAN_HOME)bin/stanc$(EXE) $(CMDSTAN_HOME)bin/print$(EXE)
	@echo ''
	@echo '--- CmdStan built ---'

##
# Clean up.  Removes Stan compiler and libraries.
##
.PHONY: clean clean-all

clean: clean-all

clean-all:
	$(RM) -r $(CMDSTAN_HOME)bin

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

