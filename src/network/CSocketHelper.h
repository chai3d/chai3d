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
    \author    Sebastien Grange
    \version   3.3.0
*/
//==============================================================================

//------------------------------------------------------------------------------
#ifndef CSocketHelperH
#define CSocketHelperH
//------------------------------------------------------------------------------
#if defined(WIN32) | defined(WIN64)
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define NOMINMAX
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment (lib,"ws2_32.lib")
#include <basetsd.h>
typedef SSIZE_T ssize_t;
typedef int  socklen_t;
typedef char brd_t;
#define cSocket_errno WSAGetLastError()
//------------------------------------------------------------------------------
#else
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <ifaddrs.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
typedef int brd_t;
#define SOCKET_ERROR -1
#define INVALID_SOCKET (~0)
#define cSocket_errno errno
#endif
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
namespace chai3d {
//------------------------------------------------------------------------------

//==============================================================================
/*!
    \file       CSocketHelper.h
    \ingroup    network

    \brief
    Implement OS-independent socket management functions.
*/
//==============================================================================

//------------------------------------------------------------------------------
/*!
    \addtogroup network
*/
//------------------------------------------------------------------------------

//@{

    //! This function initializes a TCP or UDP socket.
    int cSocketInitialize();

    //! This function configures a TCP or UDP socket blocking mode.
    int cSocketBlock(int a_socket, bool a_block);

    //! This function closes a TCP or UDP socket.
    void cSocketClose(int &a_socket);

    //! This function clears all flags on a TCP or UDP socket.
    void cSocketCleanup();

    //! This function sets the timeout associated with a TCP or UDP socket.
    int cSocketSetTimeout(int a_socket, double a_timeout);

    //! This function retrieves the timeout associated with a TCP or UDP socket.
    double cSocketGetTimeout(int a_socket);

    //! This function retrieves the local IP address.
    int cSocketGetLocalIP(int a_index, sockaddr_in *a_sin);

//@}

//------------------------------------------------------------------------------
} // namespace chai3d
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
#endif
//------------------------------------------------------------------------------
