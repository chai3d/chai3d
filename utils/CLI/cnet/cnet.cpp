//===========================================================================
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
//===========================================================================

//---------------------------------------------------------------------------
#include <iostream>
#include <iomanip>
using namespace std;
//---------------------------------------------------------------------------
#include "chai3d.h"
using namespace chai3d;
//---------------------------------------------------------------------------
typedef unsigned long int ulint;


//---------------------------------------------------------------------------
// DECLARED FUNCTIONS
//---------------------------------------------------------------------------


int strToInt(const std::string& s)
{
    std::istringstream i(s);
    int x;
    if (!(i >> x)) return 0;
    else return x;
}


// usage printer
void usage()
{
    cout << "usage: cnet [-h] [-t UDP|TCP] [-n] [-l] [-w] [-s size] [-c host] [port]" << endl;
    cout << endl;
    cout << "\t-h\tdisplay this message" << endl;
    cout << "\t-t\tselect socket type (UDP by default)" << endl;
    cout << "\t-n\tuse non-blocking socket" << endl;
    cout << "\t-l\tbind socket to loopback IP (127.0.0.1)" << endl;
    cout << "\t-s\tmessage size in [bytes] (default to 512)" << endl;
    cout << "\t-c\tclient mode, connect to given host" << endl;
    cout << "\tport\tserver port number (default to 3040)" << endl;

    exit (0);
}


// display
void updateStatistics(int msgsize)
{
    const double DISPLAY_PERIOD = 1.0;

    static ulint   msgcount  = 0;
    static double  lastTime  = 0.0;
    static double  minDelay  = DISPLAY_PERIOD;
    static double  maxDelay  = 0.0;
    static double  startTime = cPrecisionClock::getCPUTimeSeconds();

    double time, dt;

    if (msgsize > 0)
    {
        msgcount++;
    }
    time = cPrecisionClock::getCPUTimeSeconds() - startTime;
    dt = time - lastTime;
    lastTime = time;
    if (dt < minDelay)
    {
        minDelay = dt;
    }
    if (dt > maxDelay)
    {
        maxDelay = dt;
    }
    if (time > DISPLAY_PERIOD)
    {
        cout << fixed << setprecision(1) << (double)(msgcount)/(1000.0*time) << " kHz, jitter = " << (int)(1000000.0*(maxDelay-minDelay)) << " us" << endl;
        startTime = cPrecisionClock::getCPUTimeSeconds();
        lastTime = 0.0;
        msgcount = 0;
        minDelay = DISPLAY_PERIOD;
        maxDelay = 0.0;
    }
}



//===========================================================================
/*
    UTILITY:    cnet.cpp

    This utility tests the network performance in any combination of
    local/remote and TCP/UDP client/server configuration.
 */
//===========================================================================

int main (int argc, char *argv[])
{
    // as we are sending and receiving from the same socket, we need to work in blocking mode
    // otherwise the send buffer overflows and we receive an EWOULDBLOCK error
    bool block = true;

    string srvname;
    string type = "UDP";
    int msgsize = 1;
    int done = 0;
    int i;
    char *msgsrv;
    char *msgclt;
    int port = 3040;
    int size = 512;
    bool srv = true;
    bool loopback = false;

    // pretty message
    cout << endl;
    cout << "-----------------------------------" << endl;
    cout << "CHAI3D" << endl;
    cout << "Network Performance Test" << endl;
    cout << "Copyright 2003-2022" << endl;
    cout << "-----------------------------------" << endl;
    cout << endl;

    // process parameters
    if (argc < 1)
    {
        usage ();
    }
    for (i=1; i<argc; i++)
    {
        if (argv[i][0] == '-')
        {
            switch (argv[i][1])
            {
            case 'c':
                srv = false;
                if ((i < argc-1) && (argv[i+1][0] != '-'))
                {
                    i++;
                    srvname = string (argv[i]);
                }
                else usage ();
                break;
            case 's':
                if ((i < argc-1) && (argv[i+1][0] != '-'))
                {
                    i++;
                    size = strToInt (string(argv[i]));
                }
                else usage ();
                break;
            case 'l':
                loopback = true;
                break;
            case 't':
                if ((i < argc-1) && (argv[i+1][0] != '-'))
                {
                    i++;
                    type = string (argv[i]);
                }
                else usage ();
                break;
            case 'n':
                block = false;
                break;
            default:
            case 'h':
                usage ();
                break;
            }
        }
        else {
            port = strToInt (string(argv[i]));
        }
    }

    // some local info
    if (loopback) cout << "running on 127.0.0.1 (loopback adapter)" << endl;
    else          cout << "running on " << cSocket::getLocalIP() << endl;

    // generate pseudo-random message of requested size
    msgsrv = new char[size];
    msgclt = new char[size];
    for (i=0; i<size; i++)
    {
        msgsrv[i] = i%256;
        msgclt[i] = (255-i)%256;
    }


    //
    // UDP, server mode
    //

    if (type == "UDP" && srv)
    {
        cSocketUDP server;
        char header[255];

        if (server.listen(port, true, loopback) < 0)
        {
            cout << "cannot listen on " << type << " port " << port << endl;
            exit (-1);
        }
        cout << "listening on " << type << " port " << server.getLocalPort() << endl;

        // receive header containing client port and IP address from the client
        if (server.read(header, 255) <= -1)
        {
            cout << "handshake read error" << endl;
            exit (-2);
        }
        unsigned short *rport = (unsigned short*)((void*)(header));
        char *client = header+sizeof(unsigned short);
        cout << "received connection from " << client << ":" << *rport << endl << endl;

        // connect to the client
        if (server.setPeer(client, *rport, block) <= -1)
        {
            cout << "set peer failed" << endl;
            exit (-2);
        }

        // loop
        while (!done) {

            // write-read cycle
            // if non-blocking, only send once after we received something
            if (block || msgsize > 0)
            {
                if (server.send(msgsrv, size) < 0)
                {
                    cout << "send error "                    << endl;
                    done = 1;
                }
            }
            msgsize = server.read(msgclt, size);
            if (msgsize <= -1)
            {
                cout << "read error (" << msgsize << ")"   << endl;
                done = 1;
            }

            // gather some statistics and display periodically
            if (msgsize > 0) updateStatistics (msgsize);
        }
        cout << "loop terminated" << endl;
    }


    //
    // UDP, client mode
    //

    else if (type == "UDP" && !srv)
    {
        cSocketUDP client;
        unsigned short lport = port+1;
        char localIP[16];
        char header[255];

        // determine local IP
        if (loopback) sprintf (localIP, "127.0.0.1");
        else          sprintf (localIP, "%s", cSocket::getLocalIP().c_str());

        cout << "connecting to " << type << " " << srvname << ":" << port << " from " << type << " port " << lport << endl;
        if (client.connect(srvname.c_str(), port, lport, block) == 0)
        {
            cout << "connection successful" << endl << endl;

            // send header containing local port and local IP address to the server
            *(unsigned short*)(void*)(header) = lport;
            strcpy (header+sizeof(unsigned short), localIP);
            client.send(header, (int)(sizeof(unsigned short)+strlen(localIP)+1));

            // loop
            while (!done)
            {
                // read-write cycle
                // if non-blocking, only send once after we received something
                msgsize = client.read(msgsrv, size);
                if (msgsize <= -1)
                {
                    cout << "read error (" << msgsize << ")"   << endl;
                    done = 1;
                }
                if (block || msgsize > 0)
                {
                    if (client.send(msgclt, size) < 0) {
                        cout << "send error"                     << endl;
                        done = 1;
                    }
                }

                // gather some statistics and display periodically
                if (msgsize > 0) updateStatistics (msgsize);
            }
            cout << "loop terminated" << endl;
        }
        else cout << "connection failed" << endl;

    }

    //
    // TCP, server mode
    //

    else if (type == "TCP" && srv)
    {
        cSocketTCP daemon;
        cSocketTCP *server;

        if (daemon.listen(port, true, loopback) < 0)
        {
            cout << "cannot listen on port " << port << endl;
            exit (-1);
        }
        cout << "listening on " << type << " port " << daemon.getLocalPort()  << endl;
        server = daemon.accept(block, -1.0);

        if (server == NULL) cout << "connection failed" << endl;
        else
        {
            cout << "received connection from " << server->getRemoteIP() << ":" << server->getRemotePort() << endl << endl;

            // loop
            while (!done) {

                // write-read cycle
                // if non-blocking, only send once after we received something
                if (block || msgsize > 0)
                {
                    if (server->send(msgsrv, size) < 0)
                    {
                        cout << "send error"                     << endl;
                        done = 1;
                    }
                }
                msgsize = server->read(msgclt, size);
                if (msgsize <= -1)
                {
                    cout << "read error (" << msgsize << ")"   << endl;
                    done = 1;
                }

                // gather some statistics and display periodically
                if (msgsize > 0) updateStatistics (msgsize);
            }
            cout << "loop terminated" << endl;
        }
    }


    //
    // TCP, client mode
    //

    else if (type == "TCP" && !srv)
    {
        cSocketTCP client;

        cout << "connecting to " << type << " " << srvname << ":" << port << endl;
        if (client.connect(srvname.c_str(), port, block) == 0)
        {
            updateStatistics (0);
            cout << "connection successful (from port " << client.getLocalPort() << ")" << endl << endl;

            while (!done)
            {
                // read-write cycle
                // if non-blocking, only send once after we received something
                msgsize = client.read(msgsrv, size);
                if (msgsize <= -1)
                {
                    cout << "read error (" << msgsize << ")"   << endl;
                    done = 1;
                }
                if (block || msgsize > 0) {
                    if (client.send(msgclt, size) < 0)
                    {
                        cout << "send error"                     << endl;
                        done = 1;
                    }
                }

                // gather some statistics and display periodically
                if (msgsize > 0) updateStatistics (msgsize);
            }
            cout << "loop terminated" << endl;
        }
        else cout << "connection failed" << endl;

    }

    // unknown protocol
    else
    {
        cout << "unknown protocol: " << type << endl;
    }

    return 0;
}
