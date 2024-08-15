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
project (CHAI3D-BULLET)

# Version info
file (READ ${PROJECT_SOURCE_DIR}/project VERSION_FILE)
STRING (REGEX REPLACE ".*MAJOR=([0-9]+).*" "\\1" MAJOR "${VERSION_FILE}")
STRING (REGEX REPLACE ".*MINOR=([0-9]+).*" "\\1" MINOR "${VERSION_FILE}")
STRING (REGEX REPLACE ".*PATCH=([0-9]+).*" "\\1" PATCH "${VERSION_FILE}")
STRING (REGEX REPLACE ".*PRERELEASE=([0-9]+).*" "\\1" PRERELEASE "${VERSION_FILE}")
set (PROJECT_VERSION ${MAJOR}.${MINOR}.${PATCH}${PRERELEASE})

# Platform detection
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

# Output location
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY                ${PROJECT_SOURCE_DIR}/bin/${OS}-${ARCH})
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG          ${PROJECT_SOURCE_DIR}/bin/${OS}-${ARCH})
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL     ${PROJECT_SOURCE_DIR}/bin/${OS}-${ARCH})
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE        ${PROJECT_SOURCE_DIR}/bin/${OS}-${ARCH})
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${PROJECT_SOURCE_DIR}/bin/${OS}-${ARCH})

# Find dependencies.
find_package (chai3d 3.3 REQUIRED)
include_directories (${CHAI3D_INCLUDE_DIRS})
link_directories (${CHAI3D_LIBRARY_DIRS})
add_definitions (${CHAI3D_DEFINITIONS})

# Optional settings
include (${PROJECT_SOURCE_DIR}/CMakeOptions.txt OPTIONAL)

# Optional configuration (configured based on CMakeOptions.txt)
# -

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
        add_definitions (-DHAVE_GCC_DESTRUCTOR)
        set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native -Wno-deprecated -std=c++11")
        set (CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -march=native -Wno-deprecated")
    endif ()

# Linux global build options
elseif (${CMAKE_SYSTEM_NAME} MATCHES Linux)
    add_definitions (-DLINUX)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -march=native -Wno-deprecated -std=c++11")
    set (CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -fPIC -march=native -Wno-deprecated")

# Mac OS X global build options
elseif (${CMAKE_SYSTEM_NAME} MATCHES Darwin)
    add_definitions (-DMACOSX)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Qunused-arguments -Wno-deprecated -std=c++11")
    set (CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -Qunused-arguments -Wno-deprecated")
endif ()

################################################################################
#
# Static library
#
################################################################################

# Header search path
include_directories (${PROJECT_SOURCE_DIR}/src)
include_directories (${PROJECT_SOURCE_DIR}/externals/bullet/src)

# Static library source files
file (GLOB_RECURSE source RELATIVE ${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/src/*.cpp ${PROJECT_SOURCE_DIR}/src/*.h)

# Static library external dependencies source files
file (GLOB_RECURSE source_collision  RELATIVE ${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/externals/bullet/src/BulletCollision/*.cpp ${PROJECT_SOURCE_DIR}/externals/bullet/src/BulletCollision/*.h)
file (GLOB_RECURSE source_dynamics   RELATIVE ${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/externals/bullet/src/BulletDynamics/*.cpp  ${PROJECT_SOURCE_DIR}/externals/bullet/src/BulletDynamics/*.h)
file (GLOB_RECURSE source_softbody   RELATIVE ${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/externals/bullet/src/BulletSoftBody/*.cpp  ${PROJECT_SOURCE_DIR}/externals/bullet/src/BulletSoftBody/*.h)
file (GLOB_RECURSE source_linearmath RELATIVE ${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/externals/bullet/src/LinearMath/*.cpp      ${PROJECT_SOURCE_DIR}/externals/bullet/src/LinearMath/*.h)

# Build flags (put all definitions to be exported in PROJECT_DEFINITIONS)
set (PROJECT_DEFINITIONS "${PROJECT_DEFINITIONS} -DBT_USE_DOUBLE_PRECISION")
add_definitions (${PROJECT_DEFINITIONS})

# Group source files (MSVC likes this).
foreach (FILE ${source_collision} ${source_dynamics} ${source_softbody} ${source_linearmath})
    get_filename_component (PARENT_DIR "${FILE}" PATH)
    string (REGEX REPLACE "(\\./)?(src|include)/?" "" GROUP "${PARENT_DIR}")
    string (REPLACE "/" "\\" GROUP "${GROUP}")
    source_group ("${GROUP}" FILES "${FILE}")
endforeach ()

# Platform-specific source files adjustments
# -

# Internal library build flags
if (${CMAKE_BUILD_TYPE} MATCHES Debug)
    add_definitions ("-DDEBUG")
endif ()
if (${CMAKE_SIZEOF_VOID_P} EQUAL 8 AND NOT MSVC AND ${CMAKE_HOST_SYSTEM_PROCESSOR} MATCHES x86_64)
    add_definitions ("-DUSE_X86_64_ASM")
endif ()

# Static library
add_library (chai3d-Bullet STATIC ${source} ${source_collision} ${source_dynamics} ${source_softbody} ${source_linearmath} )

# Library exports
set (CHAI3D-BULLET_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/src" "${PROJECT_SOURCE_DIR}/externals/bullet/src")
set (CHAI3D-BULLET_LIBRARIES chai3d-Bullet)
set (CHAI3D-BULLET_DEFINITIONS ${PROJECT_DEFINITIONS})

################################################################################
#
# Executables
#
################################################################################

# Examples
if (EXISTS ${PROJECT_SOURCE_DIR}/examples)
    add_subdirectory (${PROJECT_SOURCE_DIR}/examples)
endif ()

################################################################################
#
# Export package
#
################################################################################

# Export package for use from the build tree.
export (TARGETS chai3d-Bullet FILE ${PROJECT_BINARY_DIR}/chai3d-BulletTargets.cmake)
export (PACKAGE chai3d-Bullet)

# Configure package.
set (CONF_INCLUDE_DIRS ${CHAI3D-BULLET_INCLUDE_DIRS})
set (CONF_DEFINITIONS  ${CHAI3D-BULLET_DEFINITIONS})

# Create package configuration and version files.
configure_file (chai3d-BulletConfig.cmake.in "${PROJECT_BINARY_DIR}/chai3d-BulletConfig.cmake" @ONLY)
configure_file (chai3d-BulletConfigVersion.cmake.in "${PROJECT_BINARY_DIR}/chai3d-BulletConfigVersion.cmake" @ONLY)
