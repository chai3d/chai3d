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
# Project-specific changes should go in Makefile.project.

# Default target
all: project

# Common settings
TOP_DIR = ../../..
ifneq (,$(wildcard $(TOP_DIR)/Makefile.common))
    include $(TOP_DIR)/Makefile.common
endif

# Directories
SRC_DIR = .

# Files
SOURCES  = $(call rwildcard,$(SRC_DIR)/,*.cpp)
INCLUDES = $(call rwildcard,$(SRC_DIR)/,*.h)
NAME     = $(notdir $(shell pwd))
TARGET   = $(BIN_DIR)/$(NAME)

# Qt files (including common resource types)
PRO        = $(call rwildcard,$(SRC_DIR)/,*.pro)
FORMS      = $(call rwildcard,$(SRC_DIR)/,*.ui)
RESOURCES  = $(call rwildcard,$(SRC_DIR)/,*.rcc)
RESOURCES += $(call rwildcard,$(SRC_DIR)/,*.qrc)
RESOURCES += $(call rwildcard,$(SRC_DIR)/,*.png)
RESOURCES += $(call rwildcard,$(SRC_DIR)/,*.ico)
RESOURCES += $(call rwildcard,$(SRC_DIR)/,*.icns)
RESOURCES += $(call rwildcard,$(SRC_DIR)/,*.qml)
RESOURCES += $(call rwildcard,$(SRC_DIR)/,*.conf)

# Tool requirement: check that qmake exists.
ifeq (, $(shell which qmake))

    # Qmake is missing.
    $(info qmake not found, skipping ${TARGET})
    TARGET :=

else

    # Qmake exists.

    # Import platform-specific settings.
    ifneq (,$(wildcard ./Makefile.$(OS)))
        include ./Makefile.$(OS)
    endif

    # Import project settings.
    ifneq (,$(wildcard ./Makefile.project))
        include ./Makefile.project
    endif

    # Import platform-specific project settings.
    ifneq (,$(wildcard ./Makefile.project.$(OS)))
        include ./Makefile.project.$(OS)
    endif

endif

# Build configuration specific flags.
ifeq ($(CFG),debug)
    QFLAGS += CONFIG+=debug
else
    QFLAGS += CONFIG+=release
    QFLAGS += CONFIG+=optimize_full
    QFLAGS += -Wnone
endif

# Build rules

project: $(TARGET)

$(TARGET): Makefile.qt.$(OS).$(ARCH).$(COMPILER) $(SOURCES) $(INCLUDES) $(FORMS) $(RESOURCES) $(LIB_STATIC) $(OBJECTS) | $(BIN_DIR)
	rm -rf $(TARGET)
	$(MAKE) -f Makefile.qt.$(OS).$(ARCH).$(COMPILER)
	$(DEPLOY)

Makefile.qt.$(OS).$(ARCH).$(COMPILER): $(PRO)
	qmake $(QFLAGS) QMAKE_CC=$(CC) QMAKE_CXX=$(CXX) QMAKE_LINK=$(CXX) OS=$(OS) ARCH=$(ARCH) COMPILER=$(COMPILER) CFG=$(CFG) -o Makefile.qt.$(OS).$(ARCH).$(COMPILER)

.PHONY: project

clean:
	rm -f $(TARGET) *~ TAGS core *.bak #*#
	-rmdir rcc uic moc