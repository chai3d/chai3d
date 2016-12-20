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
    \author    Krzysztof Kondrak
    \author    Francois Conti
    \version   3.2.0 $Rev: 2173 $
*/
//==============================================================================

//------------------------------------------------------------------------------
#ifndef OVRRenderContextH
#define OVRRenderContextH
//------------------------------------------------------------------------------
#include "math/CMaths.h"
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
namespace chai3d {
//------------------------------------------------------------------------------

//==============================================================================
/*!
    \file       OVRRenderContext.h
    \ingroup    OCULUS

    \brief
    Implements a rendering context.
*/
//==============================================================================

//==============================================================================
/*!
    \class      cOVRRenderContext

    \brief
    This class implements a rendering context using the SDL library.

    \details
    This class implements a rendering context using the SDL library.
*/
//==============================================================================
class cOVRRenderContext
{
    //--------------------------------------------------------------------------
    // CONSTRUCTOR & DESTRUCTOR:
    //--------------------------------------------------------------------------

public:

    //! Constructor of cOVRRenderContext.
    cOVRRenderContext()
    {
        m_fov = (float)chai3d::cDegToRad(75.0);
        m_nearPlane = 0.1f;
        m_farPlane = 1000.0f;
        m_scrRatio = 0.0f;
        m_width = 0;
        m_height = 0;
        m_halfWidth = 0;
        m_halfHeight = 0;
        m_left = 0.0f;
        m_right = 0.0f;
        m_bottom = 0.0f;
        m_top = 0.0f;
    }

    //! Destructor of cOVRRenderContext.
    ~cOVRRenderContext() {}


    //--------------------------------------------------------------------------
    // PUBLIC METHODS:
    //--------------------------------------------------------------------------

public:

    void init(const char *title, int x, int y, int w, int h);
    void destroy();


    //--------------------------------------------------------------------------
    // PUBLIC MEMBERS:
    //--------------------------------------------------------------------------

public:

    float m_fov;
    float m_nearPlane;
    float m_farPlane;
    float m_scrRatio;
    int   m_width;
    int   m_height;
    int   m_halfWidth;
    int   m_halfHeight;

    // ortho parameters
    float m_left;
    float m_right;
    float m_bottom;
    float m_top;

    cTransform m_modelViewProjectionMatrix; // global MVP used to orient the entire world
};

//------------------------------------------------------------------------------
} // namespace chai3d
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
#endif
//------------------------------------------------------------------------------
