//===========================================================================
/*
    Software License Agreement (BSD License)
    Copyright (c) 2003-2016, CHAI3D
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
    \version   3.2.0 $Rev: 1869 $
*/
//===========================================================================

//---------------------------------------------------------------------------
#ifndef CODEH
#define CODEH
//---------------------------------------------------------------------------

//===========================================================================
/*!
    \file       CODE.h

    \brief
    ODE module main header file.
*/
//===========================================================================

//---------------------------------------------------------------------------
//!  \defgroup   ODE   ODE Dynamics Engine
//!  \brief      Implements a CHAI3D module that supports the ODE dynamics
//!              engine.
//!  \details
//!  This module is an extension to the core CHAI3D framework that incorporates
//!  support of the Open Dynamics Engine (ODE) developed by Russel Smith.
//!
//!  For a general introduction to the ODE libraries, we recommend to review the
//!  documentation and examples that are available on the official
//!  <a href="http://www.ode.org" target="_blank">ODE website</a>.
//---------------------------------------------------------------------------

// Original ODE files
#ifndef dDOUBLE
#define dDOUBLE
#endif
#include "ode/ode.h"

// Interface classes between CHAI3D and ODE
#include "CODEWorld.h"
#include "CODEGenericBody.h"

//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
