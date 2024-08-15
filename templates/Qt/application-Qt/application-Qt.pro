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

# Project
TEMPLATE = app
DESTDIR = ../../../bin/$$OS-$$ARCH
QT = core gui widgets svg

# Qt 6 support
greaterThan(QT_MAJOR_VERSION, 5) {
    QT += openglwidgets
}

# Build settings
MOC_DIR = ./moc
UI_DIR = ./ui
RCC_DIR = ./rcc
CONFIG += c++17
win32: {
    OBJECTS_DIR += ./obj/$(Configuration)/$(Platform)
    CONFIG += windows
}
unix: {
    OBJECTS_DIR += ./obj/$$CFG/$$OS-$$ARCH-$$COMPILER
}
unix:!macx: {
    DEFINES += LINUX
}
macx: {
    DEFINES += MACOSX
    CONFIG += app_bundle sdk_no_version_check
    QMAKE_MACOSX_DEPLOYMENT_TARGET = $$OSXVER
    QMAKE_MAC_SDK = macosx$$OSXVER
    equals(ARCH,universal) {
        QMAKE_APPLE_DEVICE_ARCHS = x86_64 arm64
    }
    else {
        QMAKE_APPLE_DEVICE_ARCHS = $$ARCH
    }
}

# Compiler-specific settings
win32: {
    QMAKE_CXXFLAGS_WARN_ON -= -w34100
}
!win32: {
    QMAKE_CXXFLAGS_WARN_ON += -Wno-unused-parameter
}

# CHAI3D dependency
INCLUDEPATH += ../../../src
INCLUDEPATH += ../../../externals/Eigen
INCLUDEPATH += ../../../externals/glew/include
win32: {
    PRE_TARGETDEPS += ../../../lib/$(Configuration)/$(Platform)/chai3d.lib
    LIBS += ../../../lib/$(Configuration)/$(Platform)/chai3d.lib opengl32.lib glu32.lib
}
unix: {
    PRE_TARGETDEPS += ../../../lib/$$CFG/$$OS-$$ARCH-$$COMPILER/libchai3d.a
    LIBS += -L../../../lib/$$CFG/$$OS-$$ARCH-$$COMPILER -lchai3d
    LIBS += -L../../../externals/DHD/lib/$$OS-$$ARCH -ldrd
}
unix:!macx: {
    LIBS += -ldl -lpng -lGLU -lusb-1.0
}
macx: {
    LIBS += -framework CoreAudio -framework AudioToolbox -framework AudioUnit
}

# Resources
RESOURCES += resources.qrc
win32: RC_FILE = win32.rc
unix:!macx: { ICON = chai3d.ico }
macx: ICON = chai3d.icns

# Sources
FORMS += Interface.ui
HEADERS += Application.h Interface.h
SOURCES += main.cpp Application.cpp Interface.cpp
