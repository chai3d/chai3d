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
#include "network/CSocket.h"
#include "network/CSocketHelper.h"
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
namespace chai3d {
//------------------------------------------------------------------------------

// static members instantiation
int  cSocket::m_socketCount = 0;
bool cSocket::m_initialized = false;


//==============================================================================
/*!
    Constructor of cSocket. \n
    This method calls the global initialization function and sets sensible
    initial values for all data members.
*/
//==============================================================================
cSocket::cSocket ()
{
    initialize();

    m_socket = (int)(INVALID_SOCKET);
    m_localIP = 0;
    m_localPort = 0;
    m_remoteIP = 0;
    m_remotePort = 0;
    m_attributes = DEFAULT_ATTR;
}


//==============================================================================
/*!
    Constructor of cSocket. \n
    This method creates a base socket object around an existing socket handle
    and sets sensible initial values for all other data members.

    \param  a_socket  A valid handle to an existing socket.
*/
//==============================================================================
cSocket::cSocket (int a_socket)
{
    initialize();

    m_socket = a_socket;
    m_localIP = 0;
    m_localPort = 0;
    m_remoteIP = 0;
    m_remotePort = 0;
    m_attributes = DEFAULT_ATTR;
}


//==============================================================================
/*!
    Destructor of cSocket.
*/
//==============================================================================
cSocket::~cSocket ()
{
  close();
  cleanup();
}


//==============================================================================
/*!
    This method retrieves the local IP address. If the system is connected to
    the network, this method returns the first non-loopback IP address
    available on the system. Otherwise, the loopback adapter address
    (usually 127.0.0.1) is returned.

    \return a string containing the local IP on success, the loopback adapter
    address otherwise.
*/
//==============================================================================
std::string cSocket::getLocalIP()
{
    std::string name ("127.0.0.1");
    int res = 0;
    sockaddr_in sin;
    int index = 0;

    try
    {
        if (!m_initialized)
        {
            res = cSocketInitialize();
            m_initialized = true;
        }

        while (name.substr(0,3) == "127")
        {
            res = cSocketGetLocalIP(index++, &sin);
            if (res < 0)
            {
                break;
            }
            else
            {
                name = inet_ntoa (sin.sin_addr);
            }
        }
    }
    catch (...)
    {
        name = "127.0.0.1";
    }

    return name;
}


//==============================================================================
/*!
    This method retrieves the local port number that the socket is connected to.
    If the socket is not connected, 0 is returned.

    \return the local port number on success, 0 otherwise.
*/
//==============================================================================
int cSocket::getLocalPort()
{
    return m_localPort;
}


//==============================================================================
/*!
    This method retrieves the remote IP address. If the socket is connected to
    a remote system, this method returns the remote IP address. Otherwise, an
    illegal IP address is returned (0.0.0.0).

    \return a string containing the remote IP on success, an illegal IP otherwise.
*/
//==============================================================================
std::string cSocket::getRemoteIP()
{
    struct in_addr in;

    memset(&in, 0, sizeof(in));
    in.s_addr = (int)m_remoteIP;

    return inet_ntoa(in);
}


//==============================================================================
/*!
    This method retrieves the remote port number that the socket is connected to.
    If the socket is not connected, 0 is returned.

    \return the remote port number on success, 0 otherwise.
*/
//==============================================================================
int cSocket::getRemotePort()
{
    return m_remotePort;
}


//==============================================================================
/*!
    This method closes the socket.

    \return 0 on success, -1 otherwise.
*/
//==============================================================================
int cSocket::close()
{
    if (m_socket == (int)INVALID_SOCKET)
    {
        return SOCKET_ERROR;
    }

    cSocketClose (m_socket);

    return 0;
}


//==============================================================================
/*!
    This method reads any available data on a socket into a given buffer, but
    without emptying the buffer. The size of the buffer returned depends on the
    buffer size and the amount of available data (whichever is smaller). If the
    \ref BLOCKING attribute is set, the call blocks until data is available.
    Otherwise, the call returns immediately if no data is available.

    \param  a_output    A buffer to collect the data requested.
    \param  a_size      The size of the __a_output__ buffer in bytes.

    \return
    The number of bytes read on success,
    \ref TIMEOUT if the socket read operation timed out,
    \ref WOULDBLOCK if the socket is non-blocking and there is no data available yet,
    -1 on error.
*/
//==============================================================================
int cSocket::peek(char *a_output, unsigned int a_size)
{
    if (m_socket == INVALID_SOCKET)
    {
        return SOCKET_ERROR;
    }

    ssize_t recvlen = recv(m_socket, a_output, a_size, MSG_PEEK);

    if (recvlen == 0)
    {
        return SOCKET_ERROR;
    }

    else if (recvlen < 0)
    {
        if (cSocket_errno == EWOULDBLOCK)
        {
            if (m_attributes & BLOCKING)
            {
                return TIMEOUT;
            }
            else
            {
                return WOULDBLOCK;
            }
        }
        else
        {
            return SOCKET_ERROR;
        }
    }

    return (int)recvlen;
}


//==============================================================================
/*!
    This method reads any available data on a socket into a given string, but
    without emptying the buffer. The size of the string returned depends on the
    the amount of available data. If the \ref BLOCKING attribute is set, the call
    blocks until data is available. Otherwise, the call returns immediately if
    no data is available.

    \param  a_output    A string to collect the data requested.

    \return
    The number of bytes read on success,
    \ref TIMEOUT if the socket read operation timed out,
    \ref WOULDBLOCK if the socket is non-blocking and there is no data available yet,
    -1 on error.
*/
//==============================================================================
int cSocket::peek(std::string& a_output)
{
    if (m_socket == INVALID_SOCKET)
    {
        return SOCKET_ERROR;
    }

    ssize_t recvlen = recv(m_socket, m_buffer, BUFFERSIZE-1, MSG_PEEK);

    if (recvlen == 0)
    {
        return SOCKET_ERROR;
    }

    else if (recvlen < 0)
    {
        if (cSocket_errno == EWOULDBLOCK)
        {
            if (m_attributes & BLOCKING)
            {
                return TIMEOUT;
            }
            else
            {
                return WOULDBLOCK;
            }
        }
        else
        {
            return SOCKET_ERROR;
        }
    }

    a_output = std::string(m_buffer, recvlen);

    return (int)recvlen;
}


//==============================================================================
/*!
    This method reads any available data on a socket into a given buffer. The
    size of the buffer returned depends on the buffer size and the amount of
    available data (whichever is smaller). If the \ref BLOCKING attribute is set,
    the call blocks until data is available. Otherwise, the call returns
    immediately if no data is available.

    \param  a_output    A buffer to collect the data requested.
    \param  a_size      The size of the __a_output__ buffer in bytes.

    \return
    The number of bytes read on success,
    \ref TIMEOUT if the socket read operation timed out,
    \ref WOULDBLOCK if the socket is non-blocking and there is no data available yet,
    -1 on error.
*/
//==============================================================================
int cSocket::read(char *a_output, unsigned int a_size)
{
    if (m_socket == INVALID_SOCKET)
    {
        return SOCKET_ERROR;
    }

    ssize_t recvlen = recv(m_socket, a_output, a_size, 0);

    if (recvlen == 0)
    {
        return SOCKET_ERROR;
    }

    if (recvlen < 0)
    {
        if (cSocket_errno == EWOULDBLOCK)
        {
            if (m_attributes & BLOCKING)
            {
                return TIMEOUT;
            }
            else
            {
                return WOULDBLOCK;
            }
        }
        else
        {
            return SOCKET_ERROR;
        }
    }

    return (int)(recvlen);
}


//==============================================================================
/*!
    This method reads any available data on a socket into a given string. The
    size of the buffer returned depends on the amount of available data. If the
    \ref BLOCKING attribute is set, the call blocks until data is available.
    Otherwise, the call returns immediately if no data is available.

    \param  a_output    A string to collect the data requested.

    \return
    The number of bytes read on success,
    \ref TIMEOUT if the socket read operation timed out,
    \ref WOULDBLOCK if the socket is non-blocking and there is no data available yet,
    -1 on error.
*/
//==============================================================================
int cSocket::read(std::string& a_output)
{
    int res = read (m_buffer, BUFFERSIZE);

    if (res > 0)
    {
        a_output = std::string(m_buffer,res);
    }

    return res;
}


//==============================================================================
/*!
    This method reads a '\n' terminated line of data on a socket into a given
    buffer. The size of the buffer returned depends on the buffer size and the
    amount of available data (whichever is smaller). If the \ref BLOCKING attribute
    is set, the call blocks until data is available. Otherwise, the call returns
    immediately if no data is available. If the buffer is too small to hold a line,
    \ref cSocket::SHORTLINE is returned.

    \param  a_output    A buffer to collect the data requested.
    \param  a_size      The size of the __a_output__ buffer in bytes.

    \return
    The number of bytes read on success,
    \ref TIMEOUT if the socket read operation timed out,
    \ref WOULDBLOCK if the socket is non-blocking and there is no data available yet,
    \ref SHORTLINE if the buffer is too small to hold the line,
    -1 on error.
*/
//==============================================================================
int cSocket::readLine(char *a_output, unsigned int a_size)
{
    if (m_socket == INVALID_SOCKET)
    {
        return SOCKET_ERROR;
    }

    bool line = false;

    ssize_t recvlen = recv(m_socket, a_output, a_size, MSG_PEEK);

    if (recvlen > 0)
    {
        for (unsigned int i=0; i<a_size-1; i++)
        {
            if (a_output[i] == '\n')
            {
                recvlen = recv(m_socket, a_output, i+1, 0);
                a_output[i+1] = 0;
                line = true;
                break;
            }
        }
    }

    if (recvlen == 0) return SOCKET_ERROR;

    if (recvlen < 0)
    {
        if (cSocket_errno == EWOULDBLOCK)
        {
            if (m_attributes & BLOCKING)
            {
                return TIMEOUT;
            }
            else
            {
                return WOULDBLOCK;
            }
        }
        else
        {
            return SOCKET_ERROR;
        }
    }

    if (!line)
    {
        return SHORTLINE;
    }
    else
    {
        return (int)recvlen;
    }
}


//==============================================================================
/*!
    This method reads a '\n' terminated line of data on a socket into a given
    string. The size of the buffer returned depends on the amount of available
    data. If the \ref BLOCKING attribute is set, the call blocks until data is
    available. Otherwise, the call returns immediately if no data is available.

    \param  a_output    A string to collect the data requested.

    \return
    The number of bytes read on success,
    \ref TIMEOUT if the socket read operation timed out,
    \ref WOULDBLOCK if the socket is non-blocking and there is no data available yet,
    -1 on error.
*/
//==============================================================================
int cSocket::readLine(std::string& a_output)
{
    int res = readLine (m_buffer, BUFFERSIZE);

    if (res > 0)
    {
        a_output = std::string(m_buffer,res);
    }

    return res;
}



//==============================================================================
/*!
    This method sends data over a socket.

    \param  a_data      A buffer containing the data to send.
    \param  a_length    The data buffer length in bytes.

    \return
    The number of bytes sent on success,
    \ref TIMEOUT if the socket send operation timed out,
    \ref WOULDBLOCK if the socket is non-blocking sending would block,
     -1 on error.
*/
//==============================================================================
int cSocket::send(const char *a_data, int a_length)
{
    if (m_socket == INVALID_SOCKET)
    {
        return SOCKET_ERROR;
    }

    ssize_t sendlen = ::send(m_socket, (const char *)a_data, (int)a_length, 0);

    if (sendlen <= 0)
    {
        if (cSocket_errno == EWOULDBLOCK)
        {
            if (m_attributes & BLOCKING)
            {
                return TIMEOUT;
            }
            else
            {
                return WOULDBLOCK;
            }
        }
        else
        {
            return SOCKET_ERROR;
        }
    }

    return (int)sendlen;
}



//==============================================================================
/*!
    This method sends data over the socket.

    \param  a_data  A string containing the data to send.

    \return
    The number of bytes sent on success,
    \ref TIMEOUT if the socket send operation timed out,
    \ref WOULDBLOCK if the socket is non-blocking sending would block,
     -1 on error.
*/
//==============================================================================
int cSocket::send(std::string a_data)
{
  return send(a_data.c_str(), (int)(a_data.size()));
}


//==============================================================================
/*!
    This method sets the timeout value in seconds for the socket.

    \return
    The timeout value if set, 0 if timeout is not set, or -1 on error.
*/
//==============================================================================
double cSocket::getTimeout()
{
  return cSocketGetTimeout (m_socket);
}


//==============================================================================
/*!
    This method retrieves the timeout value in seconds for a given socket.

    \param a_timeout    The timeout value in seconds.

    \return
    0 on success, or -1 otherwise.
*/
//==============================================================================
int cSocket::setTimeout(double a_timeout)
{
  return cSocketSetTimeout (m_socket, a_timeout);
}


//==============================================================================
/*!
    This method retrieves the socket outgoing buffer size.

    \return
    The outgoing buffer size in bytes on success, -1 on failure.
*/
//==============================================================================
int cSocket::getSendSize()
{
    int size;
    socklen_t optlen = sizeof(size);

    if (m_socket == INVALID_SOCKET)
    {
        return SOCKET_ERROR;
    }

    if (getsockopt(m_socket, SOL_SOCKET, SO_SNDBUF, (char*)&size, &optlen) == -1)
    {
        return SOCKET_ERROR;
    }

    return size;
}


//==============================================================================
/*!
    This method sets the socket outgoing buffer size.

    \param a_size   Outgoing buffer size in bytes.

    \return
    0 on success, -1 otherwise.
*/
//==============================================================================
int cSocket::setSendSize(int a_size)
{
    if (m_socket == INVALID_SOCKET)
    {
        return SOCKET_ERROR;
    }

    if (setsockopt(m_socket, SOL_SOCKET, SO_SNDBUF, (char *)&a_size, sizeof(a_size)) == -1)
    {
        return SOCKET_ERROR;
    }

    return 0;
}


//==============================================================================
/*!
    This method retrieves the socket incoming buffer size.

    \return
    The incoming buffer size in bytes on success, -1 on failure.
*/
//==============================================================================
int cSocket::getRecvSize()
{
    int size;
    socklen_t optlen = sizeof(size);

    if (m_socket == INVALID_SOCKET)
    {
        return SOCKET_ERROR;
    }

    if (getsockopt(m_socket, SOL_SOCKET, SO_RCVBUF, (char*)&size, &optlen) == -1)
    {
        return SOCKET_ERROR;
    }

    return size;
}


//==============================================================================
/*!
    This method sets the socket incoming buffer size.

    \param a_size   Incoming buffer size in bytes.

    \return
    0 on success, -1 otherwise.
*/
//==============================================================================
int cSocket::setRecvSize(int a_size)
{
    if (m_socket == INVALID_SOCKET)
    {
        return SOCKET_ERROR;
    }

    if (setsockopt(m_socket, SOL_SOCKET, SO_RCVBUF, (char *)&a_size, sizeof(a_size)) == -1)
    {
        return SOCKET_ERROR;
    }

    return 0;
}


//==============================================================================
/*!
    This method retrieves the socket attributes that are shared between
    TCP and UDP configurations.

    Attributes are single bit flags contained in a single byte.
    The following attributes are available:
    \li \ref BLOCKING

    \return
    A bitwise mask describing the socket attributes.
*/
//==============================================================================
unsigned char cSocket::getBaseAttributes()
{
  return m_attributes;
}


//==============================================================================
/*!
    This method sets the socket attributes that are shared between
    TCP and UDP configurations.

    This includes the following attributes:
    \li \ref BLOCKING

    \param a_attributes     A bitwise mask describing the socket attributes.

    \return
    0 on success, -1 on failure.
*/
//==============================================================================
int cSocket::setBaseAttributes(unsigned char a_attributes)
{
    int res = 0;
    struct sockaddr_in sin;
    socklen_t len;

    if (m_socket == INVALID_SOCKET)
    {
        return SOCKET_ERROR;
    }

    // handle blocking attribute
    if (a_attributes & BLOCKING) {
        if (cSocketBlock(m_socket, true) == -1)
        {
            res = SOCKET_ERROR;
        }
        else
        {
            m_attributes |= BLOCKING;
        }
    }
    else {
        if (cSocketBlock(m_socket, false) == -1)
        {
            res = SOCKET_ERROR;
        }
        else
        {
            m_attributes &= ~BLOCKING;
        }
    }

    // get local IP and port
    len = sizeof(sin);
    getsockname(m_socket, (struct sockaddr *)&sin, &len);
    m_localIP = sin.sin_addr.s_addr;
    m_localPort = ntohs (sin.sin_port);

    // get remote IP and port
    getpeername(m_socket, (sockaddr *)&sin, (socklen_t *)&len);
    m_remoteIP = sin.sin_addr.s_addr;
    m_remotePort = ntohs (sin.sin_port);

    return res;
}


//==============================================================================
/*!
    This method initializes the socket library, and is called once automatically
    when the first socket is created.

    \return
    0 on success, -1 on failure.
*/
//==============================================================================
int cSocket::initialize()
{
    int res = 0;

    ++m_socketCount;

    if (!m_initialized)
    {
        res = cSocketInitialize ();
        m_initialized = true;
    }

    return res;
}


//==============================================================================
/*!
    This method cleans up the socket library, and is called once automatically
    when the last socket is closed.

    \return
    0 on success, -1 on failure.
*/
//==============================================================================
int cSocket::cleanup()
{
    if (m_socketCount == 0)
    {
        return 0;
    }

    if (--m_socketCount == 0)
    {
        cSocketCleanup();
        m_initialized = false;
    }

    return 0;
}


//==============================================================================
/*!
    Constructor of cSocketUDP.
*/
//==============================================================================
cSocketUDP::cSocketUDP() : cSocket()
{
  return;
}


//==============================================================================
/*!
    Destructor of cSocketUDP.
*/
//==============================================================================
cSocketUDP::~cSocketUDP()
{
  return;
}


//==============================================================================
/*!
    This method opens a UDP socket from a specific local port for listening.
    Upon successful connection, the socket can be used for reading data from any
    client sending to it. Successful connections can be restricted to the
    incoming client by using \ref cSocketUDP::setPeer().

    \param a_localPort  Local port number to listen on.
    \param a_block      Set the \ref BLOCKING attribute (__true__ by default).
    \param a_loopback   If __true__, causes the socket to bind to the loopback
                        adapter (127.0.0.1), which circumvents (most) firewall
                        limitations and warnings.

    \return
    0 on success, -1 otherwise.
*/
//==============================================================================
int cSocketUDP::listen(unsigned short a_localPort, bool a_block, bool a_loopback)
{
    struct sockaddr_in sin;

    if (m_socket != INVALID_SOCKET)
    {
        return SOCKET_ERROR;
    }

    m_socket = (int)socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (m_socket == INVALID_SOCKET)
    {
        return SOCKET_ERROR;
    }

    // set default socket attributes
    setAttributes(DEFAULT_ATTR);

    // configure address
    memset (&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port = htons (a_localPort);
    if (a_loopback)
    {
        sin.sin_addr.s_addr = htonl (INADDR_LOOPBACK);
    }
    else
    {
        sin.sin_addr.s_addr = htonl (INADDR_ANY);
    }

    // bind socket to given port
    if (bind(m_socket, (sockaddr *)&sin, sizeof(sin)) != 0)
    {
        cSocketClose (m_socket);
        return SOCKET_ERROR;
    }

    // set actual socket attributes
    if (a_block)
    {
        m_attributes |=  BLOCKING;
    }
    else
    {
        m_attributes &= ~BLOCKING;
    }
    if (setAttributes(m_attributes) < 0)
    {
        return SOCKET_ERROR;
    }

    // set default buffer sizes
    if (setSendSize(BUFFERSIZE) < 0)
    {
        return SOCKET_ERROR;
    }
    if (setRecvSize(BUFFERSIZE) < 0)
    {
        return SOCKET_ERROR;
    }

    return 0;
}


//==============================================================================
/*!
    This methods send a UDP broadcast.

    \param a_port       Target broadcast port number.
    \param a_data       Buffer containing the data to be sent.
    \param a_length     Buffer length in bytes.

    \return
    0 on success, -1 otherwise.
*/
//==============================================================================
int cSocketUDP::broadcast(unsigned short a_port, const char *a_data, int a_length)
{
    const brd_t broadcast = 1;

    struct sockaddr_in sin;

    if (m_socket != INVALID_SOCKET)
    {
        return SOCKET_ERROR;
    }

    m_socket = (int)socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (m_socket == INVALID_SOCKET)
    {
        return SOCKET_ERROR;
    }

    if (setsockopt(m_socket, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast)) == -1)
    {
        cSocketClose (m_socket);
        return SOCKET_ERROR;
    }

    memset(&sin, 0, sizeof(sin));

    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = htonl (INADDR_BROADCAST);
    sin.sin_port = htons (a_port);

    if (sendto(m_socket, a_data, a_length, 0, (struct sockaddr*)&(sin), sizeof (sin)) == -1)
    {
        cSocketClose (m_socket);
        return SOCKET_ERROR;
    }

    cSocketClose (m_socket);

    return 0;
}


//==============================================================================
/*!
    This methods send a UDP broadcast.

    \param a_port       Target broadcast port number.
    \param a_data       Buffer containing the data to be sent.

    \return
    0 on success, -1 otherwise.
*/
//==============================================================================
int cSocketUDP::broadcast(unsigned short a_port, const std::string& a_data)
{
    return broadcast(a_port, a_data.c_str(), a_data.length());
}


//==============================================================================
/*!
    This method connects a UDP socket already opened using Listen() to a specific
    peer IP/port. Upon successful connection, the socket can be used for both
    reading and sending data.

    \param a_address    Remote peer IP address.
    \param a_port       Remote peer port number.
    \param a_block      Set the \ref BLOCKING attribute (__true__ by default).

    \return
    0 on success, -1 otherwise.
*/
//==============================================================================
int cSocketUDP::setPeer(const char *a_address,
                        unsigned short a_port,
                        bool a_block)
{
    struct sockaddr_in sin;
    struct hostent *host;

    host = gethostbyname(a_address);
    if (!host)
    {
        return SOCKET_ERROR;
    }

    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = ((struct in_addr *)(host->h_addr))->s_addr;
    sin.sin_port = htons (a_port);

    if (::connect(m_socket, ((struct sockaddr *)&sin), sizeof(sin)) != 0)
    {
        cSocketClose (m_socket);
        return SOCKET_ERROR;
    }

    // set actual socket attributes
    if (a_block)
    {
        m_attributes |=  BLOCKING;
    }
    else       {
        m_attributes &= ~BLOCKING;
    }
    if (setAttributes(m_attributes) < 0)
    {
        return SOCKET_ERROR;
    }

    return 0;
}


//==============================================================================
/*!
    This methods open a UDP socket to a specific peer IP/port and from a specific
    local port. Upon successful connection, the socket can be used for both reading
    and sending data.

    \param a_address    Remote peer IP address.
    \param a_port       Remote peer port number.
    \param a_localPort  Local port number.
    \param a_block      Set the \ref BLOCKING attribute (__true__ by default)

    \return
    0 on success, -1 otherwise.
*/
//==============================================================================
int cSocketUDP::connect(const char *a_address,
                        unsigned short a_port,
                        unsigned short a_localPort,
                        bool a_block)
{
    struct sockaddr_in sin;
    struct hostent *host;

    if (strlen(a_address) < 2)
    {
        return SOCKET_ERROR;
    }

    if (m_socket != INVALID_SOCKET)
    {
        return SOCKET_ERROR;
    }

    m_socket = (int)socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (m_socket == INVALID_SOCKET)
    {
        return SOCKET_ERROR;
    }

    // set default socket attributes
    setAttributes(DEFAULT_ATTR);

    // configure address
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = htonl(INADDR_ANY);
    sin.sin_port = htons (a_localPort);

    // bind socket to given port
    if (bind(m_socket, (sockaddr *)&sin, sizeof(sin)) != 0)
    {
        cSocketClose (m_socket);
        return SOCKET_ERROR;
    }

    host = gethostbyname (a_address);
    if (!host)
    {
        cSocketClose (m_socket);
        return SOCKET_ERROR;
    }

    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = ((struct in_addr *)(host->h_addr))->s_addr;
    sin.sin_port = htons (a_port);

    if (::connect(m_socket, ((struct sockaddr *)&sin), sizeof(sin)) != 0)
    {
        cSocketClose (m_socket);
        return SOCKET_ERROR;
    }

    // set actual socket attributes
    if (a_block)
    {
        m_attributes |=  BLOCKING;
    }
    else
    {
        m_attributes &= ~BLOCKING;
    }
    if (setAttributes(m_attributes) < 0)
    {
        return SOCKET_ERROR;
    }

    // set default buffer sizes
    if (setSendSize(BUFFERSIZE) < 0)
    {
        return SOCKET_ERROR;
    }
    if (setRecvSize(BUFFERSIZE) < 0)
    {
        return SOCKET_ERROR;
    }

    return 0;
}


//==============================================================================
/*!
    This method retrieves the socket attributes.

    Attributes are single bit flags contained in a single byte.
    The following attributes are available:
    \li \ref BLOCKING

    \return
    A single byte containing the socket attributes.
*/
//==============================================================================
unsigned char cSocketUDP::getAttributes()
{
  return getBaseAttributes();
}


//==============================================================================
/*!
    This method sets the socket attributes.

    Attributes are single bit flags contained in a single byte.
    The following attributes are available:
    \li \ref BLOCKING

    \param  a_attributes    Single byte containing the desired socket attributes.

    \return
    0 on success, -1 otherwise.
*/
//==============================================================================
int cSocketUDP::setAttributes(unsigned char a_attributes)
{
  return setBaseAttributes (a_attributes);
}


//==============================================================================
/*!
    Constructor of cSocketTCP.
*/
//==============================================================================
cSocketTCP::cSocketTCP() : cSocket()
{
  return;
}


//==============================================================================
/*!
    Destructor of cSocketTCP.
    This method creates a TCP socket object around an existing socket handle.

    \param  a_socket  A valid handle to an existing socket.
*/
//==============================================================================
cSocketTCP::cSocketTCP(int a_socket) : cSocket(a_socket)
{
  return;
}


//==============================================================================
/*!
    Destructor of cSocketTCP.
*/
//==============================================================================
cSocketTCP::~cSocketTCP()
{
  return;
}


//==============================================================================
/*!
    This method attempts to connect to a remote TCP server peer at a give IP/port.
    If the connection is successful, the socket is available for reading and
    writing data to/from the peer. The system transparently chooses any available
    local port to initiate the connection to the peer.

    \param a_address    Peer remote IP address.
    \param a_port       Peer remote port number.
    \param a_block      Set the \ref BLOCKING attribute (__true__ by default).
    \param a_timeout    Set an optional timeout value in seconds. A negative value
                        means that the call will never timeout.

    \return
    0 on success, -1 otherwise.
*/
//==============================================================================
int cSocketTCP::connect(const char *a_address,
                        unsigned short a_port,
                        bool a_block,
                        double a_timeout)
{
    struct sockaddr_in sin;
    struct hostent *host;

    if (strlen(a_address) < 2)
    {
        return SOCKET_ERROR;
    }

    close();

    m_socket = (int)socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (m_socket == INVALID_SOCKET)
    {
        return SOCKET_ERROR;
    }

    // set default socket attributes
    if (a_timeout < 0)
    {
        setAttributes(DEFAULT_ATTR);
    }
    else
    {
        setAttributes(DEFAULT_ATTR & ~BLOCKING);
    }

    // configure address
    host = gethostbyname(a_address);
    if (!host)
    {
        return SOCKET_ERROR;
    }
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = ((struct in_addr *)(host->h_addr))->s_addr;
    sin.sin_port = htons(a_port);

    // attempt a connection (default system timeout)
    if (a_timeout < 0)
    {
        if (::connect(m_socket, ((struct sockaddr *)&sin), sizeof(sin)) != 0)
        {
            cSocketClose(m_socket);
            return SOCKET_ERROR;
        }
    }

    // attempt a connection (user specified timeout)
    else
    {
        if (::connect(m_socket, ((struct sockaddr *)&sin), sizeof(sin)) != 0)
        {
            // expected SOCKET_ERROR, wait for timeout
            if (cSocket_errno == EWOULDBLOCK || cSocket_errno == EINPROGRESS)
            {
                fd_set fdw, fde;
                timeval tv;
                FD_ZERO(&fdw);
                FD_ZERO(&fde);
                FD_SET(m_socket, &fdw);
                FD_SET(m_socket, &fde);
                tv.tv_sec  = (int)a_timeout;
                tv.tv_usec = (int)(1e6*(a_timeout-tv.tv_sec));
                if (select(m_socket+1, NULL, &fdw, &fde, &tv) <= 0)
                {
                    cSocketClose (m_socket);
                    return SOCKET_ERROR;
                }
            }

            // actual SOCKET_ERROR
            else
            {
                cSocketClose (m_socket);
                return SOCKET_ERROR;
            }
        }
    }

    // set actual socket attributes
    if (a_block)
    {
        m_attributes |=  BLOCKING;
    }
    else
    {
        m_attributes &= ~BLOCKING;
    }
    if (setAttributes(m_attributes) < 0)
    {
        return SOCKET_ERROR;
    }

  return 0;
}


//==============================================================================
/*!
    This method configures the socket to listen on a given port for incoming
    connections from any TCP peers. Incoming connections must be received and
    configured by \ref cSocketTCP::accept().

    \param a_port       Local port number to listen on.
    \param a_block      Set the \ref BLOCKING attribute of the subsequent
                        \ref cSocketTCP::accept() calls.
    \param a_loopback   If true, causes the socket to bind to the loopback adapter
                        (127.0.0.1), which circumvents (most) firewall limitations
                        and warnings.

    \return
    0 on success, -1 otherwise.
*/
//==============================================================================
int cSocketTCP::listen(unsigned short a_port,
                       bool a_block,
                       bool a_loopback)
{
    struct sockaddr_in sin;

    if (m_socket != INVALID_SOCKET)
    {
        return SOCKET_ERROR;
    }

    // open socket
    m_socket = (int)socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (m_socket == INVALID_SOCKET)
    {
        return SOCKET_ERROR;
    }

    // set default socket attributes
    setAttributes(DEFAULT_ATTR);

    // configure address
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port = htons(a_port);
    if (a_loopback)
    {
        sin.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    }
    else
    {
        sin.sin_addr.s_addr = htonl(INADDR_ANY);
    }

    // bind socket to given port
    if (bind(m_socket, (sockaddr *)&sin, sizeof(sin)) != 0)
    {
        cSocketClose (m_socket);
        return SOCKET_ERROR;
    }

    // listen on the given port
    if (::listen(m_socket, SOMAXCONN) == SOCKET_ERROR)
    {
        cSocketClose (m_socket);
        return SOCKET_ERROR;
    }

    // set actual socket attributes
    if (a_block)
    {
        m_attributes |=  BLOCKING;
    }
    else
    {
        m_attributes &= ~BLOCKING;
    }
    if (setAttributes(m_attributes) < 0)
    {
        return SOCKET_ERROR;
    }

    return 0;
}


//==============================================================================
/*!
    This method listens for incoming TCP connections and spawns a new socket for
    communication once the communication is established. Calls to accept must be
    preceded by a call to \ref cSocketTCP::listen(). If \ref cSocketTCP::listen()
    was passed the \ref BLOCKING attribute, each call to \ref accept() waits for
    an incoming connection. Otherwise, \ref accept() returns immediately with
    \ref WOULDBLOCK if no connection requests are pending.

    \param a_block      Set the BLOCKING attribute of the returned TCP socket
                        (__true__ by default).
    \param a_timeout    Set an optional timeout value in seconds. A negative value
                        means that the call will never timeout. This parameters is
                        ignored if the socket is non-blocking.

    \return
    A pointer to a valid \ref cSocketTCP object connected to the peer making the
    connection request if successful, NULL otherwise.
*/
//==============================================================================
cSocketTCP *cSocketTCP::accept(bool a_block, double a_timeout)
{
    fd_set fds;
    timeval tv;
    int sock;

    // if we are meant to block, wait for an imcoming connection, or timeout
    if (m_attributes | BLOCKING)
    {
        FD_ZERO(&fds);
        FD_SET(m_socket, &fds);
        tv.tv_sec = (int)a_timeout;
        tv.tv_usec = (int)(1e6*(a_timeout-tv.tv_sec));
        if (a_timeout < 0)
        {
            sock = select(m_socket+1, &fds, NULL, NULL, NULL);
        }
        else
        {
            sock = select (m_socket+1, &fds, NULL, NULL, &tv);
        }
        if (sock <= 0)
        {
            return NULL;
        }
    }

    // forced to accept any incoming connection
    sock = (int)::accept(m_socket, NULL, NULL);
    if (sock != INVALID_SOCKET)
    {
        cSocketTCP *socket = new cSocketTCP(sock);
        unsigned char attributes = m_attributes;
        if (a_block)
        {
            attributes |=  BLOCKING;
        }
        else
        {
            attributes &= ~BLOCKING;
        }
        if (socket->setAttributes(attributes) < 0)
        {
            delete socket;
            return NULL;
        }

        // set default buffer sizes
        if (socket->setSendSize(BUFFERSIZE) < 0)
        {
            delete socket;
            return NULL;
        }
        if (socket->setRecvSize(BUFFERSIZE) < 0)
        {
            delete socket;
            return NULL;
        }

        return socket;
    }

    // failure case
    return NULL;
}



//==============================================================================
/*!
    This method retrieves the socket attributes.

    Attributes are single bit flags contained in a single byte.
    The following attributes are available:
    \li \ref BLOCKING
    \li \ref NODELAY
    \li \ref REUSEADDR

    \return
    A single byte containing the socket attributes.
*/
//==============================================================================
unsigned char cSocketTCP::getAttributes()
{
  return getBaseAttributes ();
}



//==============================================================================
/*!
    This method sets the socket attributes.

    Attributes are single bit flags contained in a single byte.
    The following attributes are available:
    \li \ref BLOCKING
    \li \ref NODELAY
    \li \ref REUSEADDR

    \param  a_attributes    Single byte containing the desired socket attributes.

    \return
    0 on success, -1 otherwise.
*/
//==============================================================================
int cSocketTCP::setAttributes(unsigned char a_attributes)
{
    int optval;
    int optlen = sizeof(optval);
    int res;

    // set all other attributes
    res = setBaseAttributes(a_attributes);

    // SO_REUSEADDR
    optlen = sizeof(optval);
    if (a_attributes & REUSEADDR)
    {
        // on Windows, sockets with the REUSEADDR attribute are non-exclusively bound to their port
        // (meaning another process can bind to the same port and hijack the connection).
        // We must therefore set the Microsoft-specific SO_EXCLUSIVEADDRUSE option instead.
#if defined(WIN32) | defined(WIN64)
        optval = 1;
        if (setsockopt(m_socket, SOL_SOCKET, SO_EXCLUSIVEADDRUSE, (char *)&optval, optlen) == -1)
        {
            res = SOCKET_ERROR;
        }
        else
        {
            m_attributes |= REUSEADDR;
        }
#else
        optval = 1;
        if (setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&optval, optlen) == -1)
        {
            res = SOCKET_ERROR;
        }
        else
        {
            m_attributes |= REUSEADDR;
        }
#endif
    }
    else {
        optval = 0;
        if (setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&optval, optlen) == -1)
        {
            res = SOCKET_ERROR;
        }
        else
        {
            m_attributes &= ~REUSEADDR;
        }
    }

    // TCP_NODELAY
    if (a_attributes & NODELAY)
    {
        optval = 1;
        if (setsockopt(m_socket, IPPROTO_TCP, TCP_NODELAY, (char *)&optval, optlen) == -1)
        {
            res = SOCKET_ERROR;
        }
        else
        {
            m_attributes |= NODELAY;
        }
    }
    else {
        optval = 0;
        if (setsockopt(m_socket, IPPROTO_TCP, TCP_NODELAY, (char *)&optval, optlen) == -1)
        {
            res = SOCKET_ERROR;
        }
        else
        {
            m_attributes &= ~NODELAY;
        }
    }

    return res;
}


//------------------------------------------------------------------------------
} // namespace chai3d
//------------------------------------------------------------------------------
