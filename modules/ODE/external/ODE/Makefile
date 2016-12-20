#  Software License Agreement (BSD License)
#  Copyright (c) 2003-2013, CHAI3D.
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
#  $Author: seb $
#  $Date: 2012-05-07 20:36:47 +1200 (Mon, 07 May 2012) $
#  $Rev: 832 $


# commong settings
TOP_DIR = .
include Makefile.common

# directories
SRC_DIR = $(TOP_DIR)/ode/src
OBJ_DIR = $(TOP_DIR)/obj/$(CFG)/$(OS)-$(ARCH)-$(COMPILER)
VPATH   = $(dir $(wildcard $(SRC_DIR)/* $(SRC_DIR)/*/* $(SRC_DIR)/*/*/*))

# external dependencies
OPCODE_DIR  = $(TOP_DIR)/OPCODE
VPATH      += $(dir $(wildcard $(OPCODE_DIR)/* $(OPCODE_DIR)/*/*))

# include paths
FLAGS += -I$(INC_DIR) -I$(SRC_DIR) -I$(SRC_DIR)/joints -I$(OPCODE_DIR) -I$(OPCODE_DIR)/Ice

# main sources
SOURCES_C    = $(wildcard $(SRC_DIR)/*.c)
SOURCES_CPP  = $(wildcard $(SRC_DIR)/*.cpp)
SOURCES_CPP += $(wildcard $(SRC_DIR)/joints/*.cpp)

# external sources
SOURCES_CPP += $(wildcard $(OPCODE_DIR)/*.cpp)
SOURCES_CPP += $(wildcard $(OPCODE_DIR)/Ice/*.cpp)

# include dependencies
INCLUDES    = $(wildcard $(INC_DIR)/*.h)
INCLUDES   += $(wildcard $(SRC_DIR)/*.h)
INCLUDES   += $(wildcard $(SRC_DIR)/joints/*.h)
INCLUDES   += $(wildcard $(OPCODE_DIR)/*.h)
INCLUDES   += $(wildcard $(OPCODE_DIR)/Ice/*.h)

# objects
OBJECTS   = $(patsubst %.c,   $(OBJ_DIR)/%.o, $(SOURCES_C))
OBJECTS  += $(patsubst %.cpp, $(OBJ_DIR)/%.o, $(SOURCES_CPP))
OBJ_TREE  = $(sort $(dir $(OBJECTS)))

# build rules

all: lib

lib: $(LIB_TARGET)

$(LIB_TARGET): $(OBJECTS) | $(LIB_DIR)
	$(AR) $(ARFLAGS) $(LIB_TARGET) $?

$(OBJECTS): $(INCLUDES) | $(OBJ_TREE)

$(LIB_DIR) $(OBJ_TREE):
	mkdir -p $@

$(OBJ_DIR)/%.o : %.c
	$(CXX) $(FLAGS) $(CFLAGS) -c -o $@ $<

$(OBJ_DIR)/%.o : %.cpp
	$(CXX) $(FLAGS) $(CXXFLAGS) -c -o $@ $<

clean:
	-rm -f $(OBJECTS) $(LIB_TARGET) *~
	-rm -rf $(LIB_DIR) $(OBJ_DIR)