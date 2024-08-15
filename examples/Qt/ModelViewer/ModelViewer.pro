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

# Shared settings headers
QT          += core gui widgets
MOC_DIR     += ./moc
UI_DIR      += ./ui
RCC_DIR     += ./rcc
INCLUDEPATH += ../../../src
INCLUDEPATH += ../../../externals/Eigen
INCLUDEPATH += ../../../externals/glew/include
DEPENDPATH  += .

# Qt 6 support
greaterThan(QT_MAJOR_VERSION, 5) {
    QT += openglwidgets
}

# Platform-specific settings
unix 
{
    SYSTEM = $$system(uname -s)
    
    contains (SYSTEM, Linux) {
        OS = lin
    }
    contains (SYSTEM, Darwin) {
        OS = mac
    }
    
    TEMPLATE        = app
    TARGET          =   ../../../bin/$$OS-$$ARCH/ModelViewer
    PRE_TARGETDEPS +=   ../../../lib/$$CFG/$$OS-$$ARCH-$$COMPILER/libchai3d.a
    LIBS           += -L../../../lib/$$CFG/$$OS-$$ARCH-$$COMPILER -lchai3d
    LIBS           += -L../../../externals/DHD/lib/$$OS-$$ARCH -ldrd  
    OBJECTS_DIR    += ./obj/$$CFG/$$OS-$$ARCH-$$COMPILER
    
    contains (SYSTEM, Linux) {
        DEFINES        += LINUX
        QMAKE_CXXFLAGS += -std=c++17 -Wno-deprecated -Wno-unused-parameter -Wno-uninitialized -Wno-unused-local-typedefs
        LIBS           += -lusb-1.0 -lrt -ldl -lpng -lGLU -lX11
        ICON            = chai3d.ico  
    }
    
    contains (SYSTEM, Darwin) {
        DEFINES        += MACOSX
        QMAKE_CXXFLAGS += -std=c++17 -stdlib=libc++
        QMAKE_LFLAGS   += -stdlib=libc++    
        LIBS           += -framework CoreFoundation -framework IOKit -framework CoreServices -framework CoreAudio -framework AudioToolbox -framework AudioUnit
        CONFIG         += app_bundle
        CONFIG         += sdk_no_version_check
        ICON            = chai3d.icns
        QMAKE_MACOSX_DEPLOYMENT_TARGET = $$OSXVER
        QMAKE_MAC_SDK                  = macosx$$OSXVER
        equals(ARCH,universal) {
            QMAKE_APPLE_DEVICE_ARCHS = x86_64 arm64
        }
        else {
            QMAKE_APPLE_DEVICE_ARCHS = $$ARCH
        }
    }
}

# Sources
SOURCES   += main.cpp Application.cpp Interface.cpp
HEADERS   += Application.h Interface.h
FORMS     += Interface.ui
RESOURCES += resources.qrc
