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

################################################################################
#
# Project configuration
#
################################################################################

cmake_minimum_required (VERSION 3.0.0)
project (FreeGLUT)

# Project version
set (PROJECT_VERSION 2.0)

# OpenGL dependency
find_package (OpenGL REQUIRED)
include_directories (OPENGL_INCLUDE_DIR)

# Enforce build type.
if (NOT CMAKE_BUILD_TYPE)
    set (CMAKE_BUILD_TYPE Release CACHE STRING "Setting build mode to Release" FORCE)
endif()

################################################################################
# 
# Build products
# 
################################################################################

# Header search path
include_directories (${PROJECT_SOURCE_DIR}/include)

# Static library source files
file (GLOB_RECURSE source ${PROJECT_SOURCE_DIR}/src/*.c)

# Build flags
add_definitions (${PROJECT_DEFINITIONS} -DFREEGLUT_STATIC -DHAVE_FCNTL_H)

# Static library
add_library (freeglut STATIC ${source})

# Library dependencies and paths
set (FREEGLUT_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/include")
set (FREEGLUT_LIBRARIES freeglut ${OPENGL_LIBRARIES})

# Export to encompassing project
set (FREEGLUT_INCLUDE_DIRS ${FREEGLUT_INCLUDE_DIRS} PARENT_SCOPE)
set (FREEGLUT_LIBRARIES ${FREEGLUT_LIBRARIES} PARENT_SCOPE)

################################################################################
# 
# Export package
# 
################################################################################

# Export package for use from the build tree.
export (TARGETS freeglut FILE ${PROJECT_BINARY_DIR}/FreeGLUTTargets.cmake)
export (PACKAGE FreeGLUT)

# Package definitions
set (CONF_INCLUDE_DIRS ${FREEGLUT_INCLUDE_DIRS})
set (CONF_LIBRARIES ${FREEGLUT_LIBRARIES})

# Package configuration and version files
configure_file (FreeGLUTConfig.cmake.in "${PROJECT_BINARY_DIR}/FreeGLUTConfig.cmake" @ONLY)
configure_file (FreeGLUTConfigVersion.cmake.in "${PROJECT_BINARY_DIR}/FreeGLUTConfigVersion.cmake" @ONLY)
