################################################################################
#
#  Software License Agreement (BSD License)
#  Copyright (c) 2003-2024, CHAI3D
#  (www.chai3d.org)
#
#  All rights reserved.
#
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions
#  are met:
#
#  * Redistributions of source code must retain the above copyright
#  notice, this list of conditions and the following disclaimer.
#
#  * Redistributions in binary form must reproduce the above
#  copyright notice, this list of conditions and the following
#  disclaimer in the documentation and/or other materials provided
#  with the distribution.
#
#  * Neither the name of CHAI3D nor the names of its contributors may
#  be used to endorse or promote products derived from this software
#  without specific prior written permission.
#
#  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
#  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
#  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
#  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
#  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
#  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
#  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
#  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
#  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
#  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
#  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
#  POSSIBILITY OF SUCH DAMAGE.
#
################################################################################

# DO NOT MODIFY THIS FILE
# Project-specific changes should go in Makefile.project(.*).

# Default targets
all: products

# Project root
TOP_DIR = .

# Import common settings.
ifneq (,$(wildcard $(TOP_DIR)/Makefile.common))
    include $(TOP_DIR)/Makefile.common
endif

# Directories
SRC_DIR        = $(TOP_DIR)/src
OBJ_DIR_STATIC = $(TOP_DIR)/obj/$(CFG)/$(OS)-$(ARCH)-$(COMPILER)/static
OBJ_DIR_SHARED = $(TOP_DIR)/obj/$(CFG)/$(OS)-$(ARCH)-$(COMPILER)/shared

# Version
VERSION_H      = $(SRC_DIR)/version.h
VERSION_SCRIPT = $(TOP_DIR)/toolchains/scripts/version.sh

# Find header dependency rules
DEPS += $(call rwildcard,$(OBJ_DIR_STATIC)/,*.d)
DEPS += $(call rwildcard,$(OBJ_DIR_SHARED)/,*.d)

# Include header dependency rules if they exist,
# or include all header files if they dont.
ifneq ($(strip $(DEPS)),)
    include $(DEPS)
else
    INCLUDES += $(LIB_INCLUDES)
    INCLUDES += $(filter-out $(VERSION_H), $(call rwildcard,$(SRC_DIR)/,*.h *.hh *.hpp *.hxx))
    INCLUDES += $(VERSION_H)
endif

# Source files
VPATH      = $(SRC_DIR)
CSOURCES   = $(call rwildcard,$(SRC_DIR)/,*.c)
CCSOURCES  = $(call rwildcard,$(SRC_DIR)/,*.cc)
CPPSOURCES = $(call rwildcard,$(SRC_DIR)/,*.cpp)
CXXSOURCES = $(call rwildcard,$(SRC_DIR)/,*.cxx)

# Compiler flags
CFLAGS   += -I$(SRC_DIR)
CXXFLAGS += -I$(SRC_DIR)

# Import project settings.
ifneq (,$(wildcard $(TOP_DIR)/Makefile.project))
    include $(TOP_DIR)/Makefile.project
endif

# Import platform-specific project settings.
ifneq (,$(wildcard $(TOP_DIR)/Makefile.project.$(OS)))
    include $(TOP_DIR)/Makefile.project.$(OS)
endif

# Static library object files
OBJ_STATIC     += $(patsubst %.c, $(OBJ_DIR_STATIC)/%.o, $(CSOURCES))
OBJ_STATIC     += $(patsubst %.cc, $(OBJ_DIR_STATIC)/%.o, $(CCSOURCES))
OBJ_STATIC     += $(patsubst %.cpp, $(OBJ_DIR_STATIC)/%.o, $(CPPSOURCES))
OBJ_STATIC     += $(patsubst %.cxx, $(OBJ_DIR_STATIC)/%.o, $(CXXSOURCES))
OBJ_TREE_STATIC = $(sort $(dir $(OBJ_STATIC)))

# Shared library object files
OBJ_SHARED     += $(patsubst %.c, $(OBJ_DIR_SHARED)/%.o, $(CSOURCES))
OBJ_SHARED     += $(patsubst %.cc, $(OBJ_DIR_SHARED)/%.o, $(CCSOURCES))
OBJ_SHARED     += $(patsubst %.cpp, $(OBJ_DIR_SHARED)/%.o, $(CPPSOURCES))
OBJ_SHARED     += $(patsubst %.cxx, $(OBJ_DIR_SHARED)/%.o, $(CXXSOURCES))
OBJ_TREE_SHARED = $(sort $(dir $(OBJ_SHARED)))

# Project prerequisites
ifneq (,$(wildcard externals/Makefile))
    EXTERNALS += externals
endif

# Extensions and modules
ifneq (,$(wildcard extensions/Makefile))
    EXTENSIONS += extensions
endif
ifneq (,$(wildcard modules/Makefile))
    EXTENSIONS += modules
endif

# Executables
ifneq (,$(wildcard applications/Makefile))
    EXECUTABLES += applications
endif
ifneq (,$(wildcard examples/Makefile))
    EXECUTABLES += examples
endif
ifneq (,$(wildcard tests/Makefile))
    EXECUTABLES += tests
endif
ifneq (,$(wildcard utils/Makefile))
    EXECUTABLES += utils
endif

# Documentation
ifneq (,$(wildcard doc/Makefile))
    DOC += doc
endif

# Global targets
static: $(LIB_STATIC)
shared: $(LIB_SHARED)
lib: $(LIB)
products: $(LIB) $(EXECUTABLES)

# Version targets (only if a library is built)
ifneq (,$(wildcard $(VERSION)))
    $(INCLUDES): Makefile
    Makefile: build_version
    build_version:
        ifeq (,$(wildcard $(VERSION_SCRIPT)))
            $(info using fixed version)
            ifneq (,$(wildcard $(SRC_DIR)))
                ifeq (,$(wildcard $(VERSION_H)))
                    $(shell touch $(VERSION_H))
                endif
            endif
        else
			@$(VERSION_SCRIPT) $(VERSION) $(VERSION_H)
        endif
endif

# Build rules

.PHONY: all clean build_version lib static shared products $(EXTERNALS) $(EXTENSIONS) $(EXECUTABLES) $(DOC)

$(OBJ_STATIC): $(INCLUDES) | $(OBJ_TREE_STATIC)

$(OBJ_SHARED): $(INCLUDES) | $(OBJ_TREE_SHARED)

$(EXECUTABLES): $(LIB) $(EXTENSIONS) | $(BIN_DIR) $(EXTERNALS)

$(EXTENSIONS): $(LIB) | $(EXTERNALS)

$(LIB_DIR) $(OBJ_TREE_STATIC) $(OBJ_TREE_SHARED) $(BIN_DIR):
	@mkdir -p $@

$(EXTENSIONS) $(EXTERNALS) $(EXECUTABLES) $(DOC):
	@$(MAKE) -C $@

$(OBJ_DIR_STATIC)/%.o : %.c
	$(CC) $(CFLAGS) -fPIC -MMD -c -o $@ $<

$(OBJ_DIR_STATIC)/%.o : %.cc
	$(CXX) $(CXXFLAGS) -fPIC -MMD -c -o $@ $<

$(OBJ_DIR_STATIC)/%.o : %.cpp
	$(CXX) $(CXXFLAGS) -fPIC -MMD -c -o $@ $<

$(OBJ_DIR_STATIC)/%.o : %.cxx
	$(CXX) $(CXXFLAGS) -fPIC -MMD -c -o $@ $<

$(OBJ_DIR_SHARED)/%.o : %.c
	$(CC) $(CFLAGS) -fPIC -fno-common -MMD -c -o $@ $<

$(OBJ_DIR_SHARED)/%.o : %.cc
	$(CXX) $(CXXFLAGS) -fPIC -fno-common -MMD -c -o $@ $<

$(OBJ_DIR_SHARED)/%.o : %.cpp
	$(CXX) $(CXXFLAGS) -fPIC -fno-common -MMD -c -o $@ $<

$(OBJ_DIR_SHARED)/%.o : %.cxx
	$(CXX) $(CXXFLAGS) -fPIC -fno-common -MMD -c -o $@ $<

$(LIB_STATIC): $(OBJ_STATIC) | $(LIB_DIR) $(EXTERNALS)
	$(AR) $(ARFLAGS) $(LIB_STATIC) $^

$(LIB_SHARED): $(OBJ_SHARED) | $(LIB_DIR) $(EXTERNALS)
	$(CXX) $(CXXFLAGS) $(LIB_SHARED_OPT) $^ $(filter-out -L$(LIB_DIR),$(LDFLAGS)) $(filter-out -l$(LIB_ROOT_NAME),$(LDLIBS)) -o $(LIB_SHARED)

clean:
	@for T in $(EXTERNALS); do make -C $$T $@; done
	@for T in $(EXTENSIONS); do make -C $$T $@; done
	@for T in $(EXECUTABLES); do make -C $$T $@; done
	@for T in $(DOC); do make -C $$T $@; done
	-rm -f $(OBJ_STATIC) $(OBJ_SHARED) *~
	-rm -rf $(LIB_DIR) $(OBJ_DIR_STATIC) $(OBJ_DIR_SHARED)
