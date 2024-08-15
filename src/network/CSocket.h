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
#ifndef CSocketH
#define CSocketH
//------------------------------------------------------------------------------
#include <string>
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
namespace chai3d {
//------------------------------------------------------------------------------

//==============================================================================
/*!
    \file       CSocket.h
    \ingroup    network

    \brief
    Implements support for TCP and UDP sockets for the Internet Protocol (IP).
*/
//==============================================================================

//==============================================================================
/*!
    \class      cSocket
    \ingroup    network

    \brief
    This class implements all methods shared by TCP and UDP sockets.
*/
//==============================================================================
class cSocket
{
    //--------------------------------------------------------------------------
    // CONSTRUCTOR & DESTRUCTOR:
    //--------------------------------------------------------------------------

public:

    cSocket();
    cSocket (int a_socket);
    virtual ~cSocket ();


    //--------------------------------------------------------------------------
    // PUBLIC METHODS:
    //--------------------------------------------------------------------------

public:

    //! This method retrieves the local IP address.
    static std::string getLocalIP();

    //! This method returns the local port number that the socket is connected to.
    int getLocalPort();

    //! This method retrieves the remote IP address that the socket is connected to.
    std::string getRemoteIP();

    //! This method returns the remote port number that the socket is connected to.
    int getRemotePort();

    //! This method closes the socket.
    int close();

    //! This method reads a copy of incoming data (without removing the data).
    int peek(char *a_output, unsigned int a_size);

    //! This method reads a copy of incoming data (without removing the data).
    int peek(std::string& a_output);

    //! This method reads a incoming data.
    int read(char *a_output, unsigned int a_size);

    //! This method reads a incoming data.
    int read(std::string& a_output);

    //! This method reads an entire line of data (delimited by the '\\n' character).
    int readLine(char *a_output, unsigned int a_size);

    //! This method reads an entire line of data (delimited by the '\\n' character).
    int readLine(std::string& a_output);

    //! This method sends data through the socket.
    int send(const char *a_data, int a_length);

    //! This method sends data through the socket.
    int send(std::string a_data);

    //! This method retrieves the socket timeout in (s).
    double getTimeout();

    //! This method sets the socket timeout in (s).
    int setTimeout(double a_timeout);

    //! This method retrieves the socket outgoing buffer size.
    int getSendSize();

    //! This method sets the socket outgoing buffer size.
    int setSendSize(int a_size);

    //! This method retrieves the socket incoming buffer size.
    int getRecvSize();

    //! This method sets the socket incoming buffer size.
    int setRecvSize(int a_size);

    //! This method retrieves the socket attributes.
    virtual unsigned char getAttributes() = 0;

    //! This method sets the socket attributes.
    virtual int setAttributes(unsigned char a_attributes) = 0;


    //--------------------------------------------------------------------------
    // PUBLIC MEMBERS:
    //--------------------------------------------------------------------------

public:

    //! This constant defines the default outgoing and incoming socket buffer sizes.
    static const unsigned int BUFFERSIZE = 2048;

    //! This constant defines the attribute bit to hold the socket BLOCKING attribute.
    static const unsigned char BLOCKING = 0x01;

    //! This constant defines the attribute bit to hold the socket REUSEADDR attribute.
    static const unsigned char REUSEADDR = 0x02;

    //! This constant defines the attribute bit to hold the socket NODELAY attribute.
    static const unsigned char NODELAY = 0x04;

    //! This constant defines the default attributes for new sockets.
    static const unsigned char DEFAULT_ATTR = 0x00 | BLOCKING | REUSEADDR | ~NODELAY;

    //! This return constant is used to signify that a non-blocking socket would block.
    static const int WOULDBLOCK = -2;

    //! This return constant is used to signify that a buffer is too small to hold a line.
    static const int SHORTLINE = -3;

    //! This return constant is used to signify that a read operation on a socket has timed out.
    static const int TIMEOUT = -4;


    //--------------------------------------------------------------------------
    // PROTECTED METHODS:
    //--------------------------------------------------------------------------

protected:


    //! This method retrieves the socket attributes shared between TCP and UDP.
    unsigned char getBaseAttributes();

    //! This method sets the socket attributes shared between TCP and UDP.
    int setBaseAttributes(unsigned char a_attributes);


    //--------------------------------------------------------------------------
    // PROTECTED MEMBERS:
    //--------------------------------------------------------------------------

protected:

    //! Unique socket identifier.
    int m_socket;

    //! Socket attributes.
    unsigned char m_attributes;

    //! Socket read buffer.
    char m_buffer[BUFFERSIZE];

    //! Socket remote IP address.
    unsigned long m_remoteIP;

    //! Socket remote port number.
    unsigned int m_remotePort;

    //! Socket local IP address.
    unsigned long m_localIP;

    //! Socket local port number.
    unsigned int m_localPort;


    //--------------------------------------------------------------------------
    // PRIVATE METHODS:
    //--------------------------------------------------------------------------

private:

    //! This method initializes the socket library.
    static int initialize();

    //! This method cleans up the socket library.
    static int cleanup();


    //--------------------------------------------------------------------------
    // PRIVATE MEMBERS:
    //--------------------------------------------------------------------------

private:

    //! Number of sockets currently opened.
    static int m_socketCount;

    //! Socket library initialization flag.
    static bool m_initialized;
};



//==============================================================================
/*!
    \class      cSocketUDP
    \ingroup    network

    \brief
    This class implements all methods specific to UDP sockets.
*/
//==============================================================================
class cSocketUDP : public cSocket
{
    //--------------------------------------------------------------------------
    // CONSTRUCTOR & DESTRUCTOR:
    //--------------------------------------------------------------------------

public:

    //! Constructor of cSocketUDP.
    cSocketUDP();

    //! Destructor of cSocketUDP.
    virtual ~cSocketUDP();


    //--------------------------------------------------------------------------
    // PUBLIC METHODS:
    //--------------------------------------------------------------------------

public:

    //! This method listens for incoming packets on a given port
    int listen(unsigned short a_lport, bool a_block = true, bool a_loopback = false);

    //! This method broadcast a packet to a given port
    int broadcast(unsigned short a_port, const char *a_data, int a_length);

    //! This method broadcast a packet to a given port
    int broadcast(unsigned short a_port, const std::string& a_data);

    //! This method assigns a peer (remote) socket to the current socket
    int setPeer(const char *a_address, unsigned short a_port, bool a_block = true);

    //! This method connects to a given remote socket
    int connect(const char *a_address, unsigned short a_port, unsigned short a_localport, bool a_block = true);

    //! This method retrieves the socket attributes
    unsigned char getAttributes();

    //! This method sets the socket attributes
    int setAttributes(unsigned char a_attributes);
};



//==============================================================================
/*!
    \class      cSocketTCP
    \ingroup    network

    \brief
    This class implements all methods specific to TCP sockets.
*/
//==============================================================================
class cSocketTCP : public cSocket
{
    //--------------------------------------------------------------------------
    // CONSTRUCTOR & DESTRUCTOR:
    //--------------------------------------------------------------------------

public:

    //! Constructor of cSocketTCP.
    cSocketTCP();

    //! Constructor of cSocketTCP using an already opened socket
    cSocketTCP(int socket);

    //! Destructor of cSocketTCP.
    virtual ~cSocketTCP();


    //--------------------------------------------------------------------------
    // PUBLIC METHODS:
    //--------------------------------------------------------------------------

public:

    //! This method connects to a remote server
    int connect(const char *a_address, unsigned short a_port, bool a_block = true, double a_timeout = -1.0);

    //! This method listens for incoming client connections
    int listen(unsigned short a_port, bool a_block = true, bool a_loopback = false);

    //! This method accepts connections from incoming client connection requests
    cSocketTCP* accept(bool a_block = true, double a_timeout = -1.0);

    //! This method retrieves the socket attributes
    unsigned char getAttributes();

    //! This method sets the socket attributes
    int setAttributes(unsigned char a_attributes);
};

//------------------------------------------------------------------------------
} // namespace chai3d
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
#endif
//------------------------------------------------------------------------------
