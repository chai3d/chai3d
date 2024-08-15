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

cmake_minimum_required (VERSION 3.0.0)
project (application-GLFW)

# chai3d dependency
find_package (chai3d 3.3 REQUIRED)
include_directories (${CHAI3D_INCLUDE_DIRS})
link_directories (${CHAI3D_LIBRARY_DIRS})
add_definitions (${CHAI3D_DEFINITIONS})

# GLFW dependency
find_package (GLFW REQUIRED)
include_directories (${GLFW_INCLUDE_DIRS})

# platform detection
if (${CMAKE_SYSTEM_NAME} MATCHES Windows)
    set (OS "win")
    if (${CMAKE_CL_64})
        set (ARCH "x64")
    else ()
        set (ARCH "Win32")
    endif ()
elseif (${CMAKE_SYSTEM_NAME} MATCHES Linux)
    set (OS "lin")
    set (ARCH "${CMAKE_SYSTEM_PROCESSOR}")
elseif (${CMAKE_SYSTEM_NAME} MATCHES Darwin)
    set (OS "mac")
    set (ARCH ${CMAKE_HOST_SYSTEM_PROCESSOR})
endif ()

# Enforce build type.
if (NOT CMAKE_BUILD_TYPE)
    set (CMAKE_BUILD_TYPE Release CACHE STRING "Setting build mode to Release" FORCE)
endif()

# Set output location.
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY                ${CHAI3D_SOURCE_DIR}/bin/${OS}-${ARCH})
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG          ${CHAI3D_SOURCE_DIR}/bin/${OS}-${ARCH})
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL     ${CHAI3D_SOURCE_DIR}/bin/${OS}-${ARCH})
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE        ${CHAI3D_SOURCE_DIR}/bin/${OS}-${ARCH})
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CHAI3D_SOURCE_DIR}/bin/${OS}-${ARCH})

# Windows global build options
if (${CMAKE_SYSTEM_NAME} MATCHES Windows)

    # VisualStudio compiler
    if (MSVC)
        add_definitions (-D_CRT_SECURE_NO_DEPRECATE)
        set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc /MP")
        if (${CMAKE_CL_64})
            add_definitions (-DWIN64)
        else ()
            add_definitions (-DWIN32)
        endif ()
   
    # MinGW compiler
    elseif (MINGW)
        add_definitions (-DWIN32)
        set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native -Wno-deprecated -std=c++11")
    endif ()

# Linux global build options
elseif (${CMAKE_SYSTEM_NAME} MATCHES Linux)

    add_definitions (-DLINUX)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native -Wno-deprecated -std=c++11")

# Mac OS X global build options
elseif (${CMAKE_SYSTEM_NAME} MATCHES Darwin)

    add_definitions (-DMACOSX)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Qunused-arguments -Wno-deprecated -std=c++11")

endif ()

# Target executable
add_executable (${PROJECT_NAME} application-GLFW.cpp)
target_link_libraries (${PROJECT_NAME} ${CHAI3D_LIBRARIES} ${GLFW_LIBRARIES})

# OS specific adjustments
if (${CMAKE_SYSTEM_NAME} MATCHES Darwin)
    add_custom_command (TARGET ${PROJECT_NAME} POST_BUILD COMMAND Rez -append ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/../resources/icons/chai3d.rsrc -o ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${PROJECT_NAME} COMMAND SetFile -a C ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${PROJECT_NAME} VERBATIM)
endif ()
