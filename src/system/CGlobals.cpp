//==============================================================================
/*
    Software License Agreement (BSD License)
    Copyright (c) 2003-2024, CHAI3D
    (www.chai3d.org)

    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.

    * Redistributions in binary form must reproduce the above
    copyright notice, this list of conditions and the following
    disclaimer in the documentation and/or other materials provided
    with the distribution.

    * Neither the name of CHAI3D nor the names of its contributors may
    be used to endorse or promote products derived from this software
    without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
    "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
    LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
    FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
    COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
    INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
    BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
    CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
    LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
    ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE. 

    \author    <http://www.chai3d.org>
    \author    Francois Conti
    \version   3.3.0
*/
//==============================================================================

//------------------------------------------------------------------------------
#include <algorithm>
using namespace std;
//------------------------------------------------------------------------------
#include "version.h"
#include "system/CGlobals.h"
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
namespace chai3d {
//------------------------------------------------------------------------------

//==============================================================================
/*!
    This function suspends the execution of the current thread for a specified 
    interval defined in milliseconds.

    \param  a_interval  Time interval defined in milliseconds.
*/
//==============================================================================
void cSleepMs(const unsigned int a_interval)
{
#if defined(WIN32) | defined(WIN64)
    Sleep(a_interval);
#endif

#if defined(LINUX) | defined (MACOSX)
    struct timespec t;
    t.tv_sec  = a_interval/1000;
    t.tv_nsec = (a_interval-t.tv_sec*1000)*1000000;
    nanosleep (&t, NULL);
#endif
}

//==============================================================================
/*!
    This function returns the absolute path of the current executable linked
    agains the CHAI3D library. On Mac OS X, this returns the path to the bundle
    (as opposed to the path to the actual executable nested within the bundle).

    \return Path name.
*/
//==============================================================================
std::string cGetCurrentPath()
{

#if defined(WIN32) | defined(WIN64)
    char buffer[MAX_PATH];
    GetModuleFileName(NULL, buffer, MAX_PATH);
    string path = string(buffer).substr(0, string(buffer).find_last_of("\\/"));
    if (path.length() <= 0) path = ".";
    return path.append("/");
#endif

#if defined(LINUX)
    char symlink[PATH_MAX];
    char buffer[PATH_MAX];
    sprintf(symlink, "/proc/%d/exe", getpid());
    buffer[std::min(readlink(symlink, buffer, PATH_MAX), (ssize_t)(PATH_MAX-1))] = '\0';
    return (string(buffer).substr(0, string(buffer).find_last_of("\\/"))).append("/");
#endif

#if defined (MACOSX)
    CFURLRef bundleURL;
    UInt8 bundlePath[PATH_MAX];
    memset(bundlePath, 0x00, PATH_MAX);
    bundleURL = CFBundleCopyBundleURL(CFBundleGetMainBundle());
    CFURLGetFileSystemRepresentation(bundleURL, 1, bundlePath, PATH_MAX);
    CFRelease(bundleURL);
    return string(reinterpret_cast<char*>(bundlePath)) + "/";
#endif

}

//==============================================================================
/*!
    This function returns the version of the CHAI3D library as a string.

    \return Version string.
*/
//==============================================================================
const std::string cGetVersion()
{
    return CHAI3D_VERSION;
}

//==============================================================================
/*!
    This function returns the versions of the CHAI3D library as defined by the
    semantic versioning 2.0 guidelines (semver.org).

    \param  a_major The major version number.
    \param  a_minor The minor version number.
    \param  a_patch The patch number.
*/
//==============================================================================
void cGetVersion(int& a_major,
                 int& a_minor,
                 int& a_patch)
{
    a_major = CHAI3D_VERSION_MAJOR;
    a_minor = CHAI3D_VERSION_MINOR;
    a_patch = CHAI3D_VERSION_PATCH;
}

//------------------------------------------------------------------------------
} // namespace chai3d
//------------------------------------------------------------------------------
