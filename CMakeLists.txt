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
# Configuration
#
################################################################################

cmake_minimum_required (VERSION 3.0.0)
project (CHAI3D)

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

# OpenGL dependency
find_package (OpenGL REQUIRED)
include_directories (OPENGL_INCLUDE_DIR)

# On non-Windows, DHD dependency
if (NOT ${CMAKE_SYSTEM_NAME} MATCHES Windows)
    include_directories (${PROJECT_SOURCE_DIR}/externals/DHD/include)
    if(${CMAKE_GENERATOR} STREQUAL Xcode)
        set (DHD_LIBRARY_DIRS ${PROJECT_SOURCE_DIR}/externals/DHD/lib/${OS})
    else()
        set (DHD_LIBRARY_DIRS ${PROJECT_SOURCE_DIR}/externals/DHD/lib/${OS}-${ARCH})
    endif()
    link_directories (${DHD_LIBRARY_DIRS})
endif ()

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
include_directories (${PROJECT_SOURCE_DIR}/externals/Eigen)
include_directories (${PROJECT_SOURCE_DIR}/externals/glew/include)
include_directories (${PROJECT_SOURCE_DIR}/externals/giflib/include)
include_directories (${PROJECT_SOURCE_DIR}/externals/lib3ds/include)
include_directories (${PROJECT_SOURCE_DIR}/externals/libjpeg/include)
include_directories (${PROJECT_SOURCE_DIR}/externals/libpng/include)
include_directories (${PROJECT_SOURCE_DIR}/externals/minimp3/include)
include_directories (${PROJECT_SOURCE_DIR}/externals/openal/include)
include_directories (${PROJECT_SOURCE_DIR}/externals/openal/OpenAL32/Include)
include_directories (${PROJECT_SOURCE_DIR}/externals/openal/Alc)
include_directories (${PROJECT_SOURCE_DIR}/externals/pugixml/include)
include_directories (${PROJECT_SOURCE_DIR}/externals/STB/include)
include_directories (${PROJECT_SOURCE_DIR}/externals/theoraplayer/include/theoraplayer)
include_directories (${PROJECT_SOURCE_DIR}/externals/theoraplayer/externals/ogg/include)
include_directories (${PROJECT_SOURCE_DIR}/externals/theoraplayer/externals/theora/include)
include_directories (${PROJECT_SOURCE_DIR}/externals/theoraplayer/externals/vorbis/include)
include_directories (${PROJECT_SOURCE_DIR}/externals/theoraplayer/externals/vorbis/lib)
include_directories (${PROJECT_SOURCE_DIR}/externals/theoraplayer/src/Theora)
include_directories (${PROJECT_SOURCE_DIR}/externals/RtMidi)

# Static library source files
file (GLOB_RECURSE source RELATIVE ${CHAI3D_SOURCE_DIR} ${CHAI3D_SOURCE_DIR}/src/*.cpp ${CHAI3D_SOURCE_DIR}/src/*.h)

# Static library external dependencies source files
file (GLOB_RECURSE source_eigen     RELATIVE ${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/externals/Eigen/Eigen/*)
file (GLOB_RECURSE source_gif       RELATIVE ${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/externals/giflib/*.c       ${PROJECT_SOURCE_DIR}/externals/giflib/*.h)
file (GLOB_RECURSE source_glew      RELATIVE ${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/externals/glew/*.c         ${PROJECT_SOURCE_DIR}/externals/glew/*.h)
file (GLOB_RECURSE source_3ds       RELATIVE ${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/externals/lib3ds/*.c       ${PROJECT_SOURCE_DIR}/externals/lib3ds/*.h)
file (GLOB_RECURSE source_jpeg      RELATIVE ${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/externals/libjpeg/*.c      ${PROJECT_SOURCE_DIR}/externals/libjpeg/*.h)
file (GLOB_RECURSE source_png       RELATIVE ${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/externals/libpng/*.c       ${PROJECT_SOURCE_DIR}/externals/libpng/*.h)
file (GLOB_RECURSE source_pugixml   RELATIVE ${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/externals/pugixml/*.c      ${PROJECT_SOURCE_DIR}/externals/pugixml/*.h)
file (GLOB_RECURSE source_openal    RELATIVE ${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/externals/openal/*.c       ${PROJECT_SOURCE_DIR}/externals/openal/*.h)
file (GLOB_RECURSE source_theora    RELATIVE ${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/externals/theoraplayer/*.c ${PROJECT_SOURCE_DIR}/externals/theoraplayer/*.cpp ${PROJECT_SOURCE_DIR}/externals/theoraplayer/*.h)
file (GLOB_RECURSE source_rtmidi    RELATIVE ${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/externals/RtMidi/*.cpp     ${PROJECT_SOURCE_DIR}/externals/RtMidi/*.h)

# Platform-specific source files adjustments
if (${CMAKE_SYSTEM_NAME} MATCHES Windows)
    file (GLOB_RECURSE source_theora_exclude RELATIVE ${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/externals/theoraplayer/externals/theora/lib/x86/*.c)
else ()
    file (GLOB_RECURSE source_theora_exclude RELATIVE ${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/externals/theoraplayer/externals/theora/lib/x86_vc/*.c)
endif ()
list (REMOVE_ITEM source_theora ${source_theora_exclude})

# Group source files (MSVC likes this).
foreach (FILE ${source} ${source_eigen} ${source_gif} ${source_glew} ${source_3ds} ${source_jpeg} ${source_png} ${source_pugixml} ${source_openal} ${source_theora} ${source_rtmidi})
    get_filename_component (PARENT_DIR "${FILE}" PATH)
    string (REGEX REPLACE "(\\./)?(src|include)/?" "" GROUP "${PARENT_DIR}")
    string (REPLACE "/" "\\" GROUP "${GROUP}")
    source_group ("${GROUP}" FILES "${FILE}")
endforeach ()

# Build flags (put all definitions to be exported in PROJECT_DEFINITIONS)
set (PROJECT_DEFINITIONS "${PROJECT_DEFINITIONS}")
add_definitions (${PROJECT_DEFINITIONS} -DGLEW_STATIC -DAL_ALEXT_PROTOTYPES -DAL_BUILD_LIBRARY -DAL_LIBTYPE_STATIC -D__THEORA -D_LIB -DPNG_ARM_NEON_OPT=0)
if (${CMAKE_SYSTEM_NAME} MATCHES Windows)
    add_definitions (${PROJECT_DEFINITIONS} -D__WINDOWS_MM__)
elseif (${CMAKE_SYSTEM_NAME} MATCHES Linux)
    add_definitions (${PROJECT_DEFINITIONS} -D__LINUX_ALSA__)
elseif (${CMAKE_SYSTEM_NAME} MATCHES Darwin)
    add_definitions (${PROJECT_DEFINITIONS} -D__MACOSX_CORE__)
endif ()

# Static library
add_library (chai3d STATIC ${source} ${source_eigen} ${source_gif} ${source_glew} ${source_3ds} ${source_jpeg} ${source_png} ${source_pugixml} ${source_openal} ${source_theora})

# Library exports
set (CHAI3D_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/src" "${PROJECT_SOURCE_DIR}/externals/Eigen" "${PROJECT_SOURCE_DIR}/externals/glew/include" "${OPENGL_INCLUDE_DIR}")
set (CHAI3D_LIBRARIES chai3d ${OPENGL_LIBRARIES})
set (CHAI3D_DEFINITIONS ${PROJECT_DEFINITIONS})
if (NOT ${CMAKE_SYSTEM_NAME} MATCHES Windows)
    set (CHAI3D_LIBRARIES ${CHAI3D_LIBRARIES} drd)
    set (CHAI3D_LIBRARY_DIRS ${CHAI3D_LIBRARY_DIRS} ${DHD_LIBRARY_DIRS})
else ()
    set (CHAI3D_LIBRARIES ${CHAI3D_LIBRARIES} winmm)
endif ()
if (${OS} MATCHES lin)
    set (CHAI3D_LIBRARIES ${CHAI3D_LIBRARIES} usb-1.0 rt pthread dl asound)
elseif (${OS} MATCHES mac)
    set (CHAI3D_LIBRARIES ${CHAI3D_LIBRARIES} "-framework CoreFoundation" "-framework IOKit" "-framework CoreAudio" "-framework AudioToolbox" "-framework CoreMIDI" "-framework Cocoa")
endif ()
set (CHAI3D_SOURCE_DIR ${PROJECT_SOURCE_DIR})

################################################################################
#
# Optional extras
#
################################################################################

# GLFW
if (EXISTS ${PROJECT_SOURCE_DIR}/extras/GLFW)
    add_subdirectory (${PROJECT_SOURCE_DIR}/extras/GLFW)
endif ()

################################################################################
#
# Executables
#
################################################################################

# Examples
if (EXISTS ${PROJECT_SOURCE_DIR}/examples)
    add_subdirectory (${PROJECT_SOURCE_DIR}/examples)
endif ()

# Utilities
if (EXISTS ${PROJECT_SOURCE_DIR}/utils)
    add_subdirectory (${PROJECT_SOURCE_DIR}/utils)
endif ()

################################################################################
#
# Export package
#
################################################################################

# Export package for use from the build tree.
export (TARGETS chai3d FILE ${PROJECT_BINARY_DIR}/CHAI3DTargets.cmake)
export (PACKAGE chai3d)

# Package definitions
set (CONF_INCLUDE_DIRS ${CHAI3D_INCLUDE_DIRS})
set (CONF_LIBRARIES    ${CHAI3D_LIBRARIES})
set (CONF_LIBRARY_DIRS ${CHAI3D_LIBRARY_DIRS})
set (CONF_DEFINITIONS  ${CHAI3D_DEFINITIONS})
set (CONF_SOURCE_DIR   ${CHAI3D_SOURCE_DIR})

# Package configuration and version files
configure_file (chai3dConfig.cmake.in "${PROJECT_BINARY_DIR}/chai3dConfig.cmake" @ONLY)
configure_file (chai3dConfigVersion.cmake.in "${PROJECT_BINARY_DIR}/chai3dConfigVersion.cmake" @ONLY)
