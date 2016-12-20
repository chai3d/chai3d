//==============================================================================
/*
    Software License Agreement (BSD License)
    Copyright (c) 2003-2016, CHAI3D.
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
    \version   3.2.0 $Rev: 2015 $
*/
//==============================================================================



//---------------------------------------------------------------------------
#ifndef CVREPH
#define CVREPH
//---------------------------------------------------------------------------

//===========================================================================
/*!
    \file v_repExtCHAI3D.h

    \brief
    <b> V-REP Module </b> \n
    Main Header File.
*/
//===========================================================================

//---------------------------------------------------------------------------
/*!  \defgroup V-REP CHAI3D plugin for V-REP
//   \brief Shared library for use by V-REP (from <a href="http://www.coppeliarobotics.com" target="_blank">Coppelia Robotics</a>).
//
//  Please refer to the \ref index "introduction" for more information on
//  how to use the CHAI3D plugin for V-REP.
*/
//---------------------------------------------------------------------------

#ifdef _WIN32
    #define VREP_DLLEXPORT extern "C" __declspec(dllexport)
#endif
#if defined (__linux) || defined (__APPLE__)
    #define VREP_DLLEXPORT extern "C"
#endif

VREP_DLLEXPORT unsigned char v_repStart   (void* reservedPointer, int reservedInt);
VREP_DLLEXPORT void          v_repEnd     ();
VREP_DLLEXPORT void*         v_repMessage (int message, int* auxiliaryData, void* customData, int* replyData);

//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
