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
#include "network/CSocketHelper.h"
//------------------------------------------------------------------------------
using namespace std;
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
namespace chai3d {
//------------------------------------------------------------------------------


//==============================================================================
/*!
    This function initializes a TCP or UDP socket. \n

    \return 0 on success, -1 otherwise.
*/
//==============================================================================
int cSocketInitialize()
{
#if defined(WIN32) || defined(WIN64)

    WORD version_wanted = MAKEWORD(2, 2);
    WSADATA wsaData;
    if (WSAStartup(version_wanted, &wsaData) != 0) return -1;
    if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) return -1;

#else

    void (*handler)(int);
    handler = signal(SIGPIPE, SIG_IGN);
    if (handler != SIG_DFL) {
        signal(SIGPIPE, handler);
    }

#endif

    return 0;
}


//==============================================================================
/*!
    This function configures a TCP or UDP socket blocking mode. \n

    \param  a_socket    The index of the socket to configure.
    \param  a_block     __true__ to make the socket blocking, __false__ otherwise.

    \return 0 on success, SOCKET_ERROR otherwise.
*/
//==============================================================================
int cSocketBlock(int a_socket, bool a_block)
{
    unsigned long arg = (a_block) ? 0 : 1;

#if defined(WIN32) || defined(WIN64)
    return ioctlsocket (a_socket, FIONBIO, &arg);
#else
    return ioctl (a_socket, FIONBIO, &arg);
#endif
}


//==============================================================================
/*!
    This function closes a TCP or UDP socket. \n

    \param  a_socket    The index of the socket to configure.
*/
//==============================================================================
void cSocketClose(int &a_socket)
{
#if defined(WIN32) || defined(WIN64)
    shutdown (a_socket, SD_BOTH);
    closesocket (a_socket);
#else
    close (a_socket);
#endif

    // invalidate closed socket
    a_socket = (int)(-1);
}


//==============================================================================
/*!
    This function clears all flags on a TCP or UDP socket. \n
*/
//==============================================================================
void cSocketCleanup()
{
#if defined(WIN32) || defined(WIN64)

    if (WSACleanup() == SOCKET_ERROR) {
        if (WSAGetLastError() == WSAEINPROGRESS) {
            WSACancelBlockingCall();
            WSACleanup();
        }
    }

#else

    void (*handler)(int);
    handler = signal(SIGPIPE, SIG_DFL);
    if (handler != SIG_IGN) {
        signal(SIGPIPE, handler);
    }

#endif
}


//==============================================================================
/*!
    This function sets the timeout associated with a TCP or UDP socket. \n

    \param  a_socket    The index of the socket to configure.
    \param  a_timeout   The desired socket timeout in (s).

    \note
    A value of __a_timeout__ of 0 indicates that the socket will not time out.

    \return 0 on success, -2 otherwise.
*/
//==============================================================================
int cSocketSetTimeout(int a_socket, double a_timeout)
{
#if defined(WIN32) || defined(WIN64)

    DWORD t = (DWORD)(a_timeout*1e3);
    if (setsockopt (a_socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)(&t), sizeof(DWORD)) == -1) return -1;
    if (setsockopt (a_socket, SOL_SOCKET, SO_SNDTIMEO, (const char*)(&t), sizeof(DWORD)) == -1) return -1;

#else

    timeval t;
    t.tv_sec  = a_timeout;
    t.tv_usec = 1e6*(a_timeout - t.tv_sec);
    if (setsockopt (a_socket, SOL_SOCKET, SO_RCVTIMEO, &t, sizeof(t)) == -1) return -1;
    if (setsockopt (a_socket, SOL_SOCKET, SO_SNDTIMEO, &t, sizeof(t)) == -1) return -1;

#endif

    return 0;
}


//==============================================================================
/*!
    This function retrieves the timeout associated with a TCP or UDP socket. \n

    \param  a_socket    The index of the socket to configure.

    \return The timeout associated with the given __a_socket__.
*/
//==============================================================================
double cSocketGetTimeout(int a_socket)
{
#if defined(WIN32) || defined(WIN64)

    DWORD     t;
    socklen_t length = sizeof(DWORD);
    if (getsockopt (a_socket, SOL_SOCKET, SO_RCVTIMEO, (char*)(&t), &length) == -1) return -1;
    if (getsockopt (a_socket, SOL_SOCKET, SO_SNDTIMEO, (char*)(&t), &length) == -1) return -1;
    return (double)t/1e3;

#else

    timeval t;
    socklen_t length = sizeof(t);
    if (getsockopt (a_socket, SOL_SOCKET, SO_RCVTIMEO, (char*)(&t), &length) == -1) return -1;
    if (getsockopt (a_socket, SOL_SOCKET, SO_SNDTIMEO, (char*)(&t), &length) == -1) return -1;
    return t.tv_sec+t.tv_usec*1e-6;

#endif
}


//==============================================================================
/*!
    This function retrieves the local IP address. \n

    \param  a_index    The index of the network interface (NIC) to query in the system.
    \param  a_sin      A pointer to a structure that will hold the IP address on success.

    \return 0 on success, -1 otherwise.
*/
//==============================================================================
int cSocketGetLocalIP(int a_index, sockaddr_in *a_sin)
{
#if defined(WIN32) || defined(WIN64)

    INTERFACE_INFO ilst[20];
    DWORD          nbytes;

    SOCKET sd = WSASocket(AF_INET, SOCK_DGRAM, IPPROTO_UDP, 0, 0, 0);
    if (sd == SOCKET_ERROR) return -1;

    if (WSAIoctl(sd, SIO_GET_INTERFACE_LIST, NULL, 0, &ilst, sizeof(ilst), &nbytes, 0, 0) == SOCKET_ERROR) return -1;
    int numif = nbytes / sizeof(INTERFACE_INFO);

    if (a_index < 0 || a_index >= numif) return -1;

    memcpy (a_sin, (sockaddr_in *)(&(ilst[a_index].iiAddress)), sizeof(sockaddr_in));

    return 0;

#else

    int             i = 0;
    struct ifaddrs *ifAddrStruct;
    struct ifaddrs *ias;

    if (getifaddrs (&ias) < 0) {
        freeifaddrs (ias);
        return -1;
    }

    // store original pointer before we increment it
    ifAddrStruct = ias;

    // iterate through network interfaces
    while (ifAddrStruct != NULL && ifAddrStruct->ifa_addr->sa_family != AF_INET) ifAddrStruct = ifAddrStruct->ifa_next;
    if (a_index > 0) do {
        ifAddrStruct = ifAddrStruct->ifa_next;
        while (ifAddrStruct != NULL && ifAddrStruct->ifa_addr->sa_family != AF_INET) ifAddrStruct = ifAddrStruct->ifa_next;
        ++i;
    } while (i != a_index);

    // report success if appropriate
    if (ifAddrStruct != NULL) {
        memcpy (a_sin, (sockaddr_in *)(ifAddrStruct->ifa_addr), sizeof(sockaddr_in));
        freeifaddrs (ias);
        return 0;
    }

    // report failure
    freeifaddrs (ias);
    return -2;

#endif
}


//------------------------------------------------------------------------------
} // namespace chai3d
//------------------------------------------------------------------------------
