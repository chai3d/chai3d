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
    \author    Dan Morris
    \version   3.3.0
*/
//==============================================================================

//------------------------------------------------------------------------------
#ifndef CViewportH
#define CViewportH
//------------------------------------------------------------------------------
#include "display/CCamera.h"
#include "display/CFrameBuffer.h"
#include "widgets/CViewPanel.h"
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
namespace chai3d {
//------------------------------------------------------------------------------

//==============================================================================
/*!
    \file       CViewport.h

    \brief
    Implementation of a camera.
*/
//==============================================================================

//==============================================================================
/*!
    \class      cViewport
    \ingroup    display

    \brief
    This class implements a viewport.

    \details
    viewport implements a viewport.
*/
//==============================================================================
class cViewport : public cGenericObject
{
    //--------------------------------------------------------------------------
    // CONSTRUCTOR & DESTRUCTOR:
    //--------------------------------------------------------------------------

public:

    //! Constructor of cViewport
    cViewport(cCamera* a_camera = nullptr, double a_contentScaleX = 1.0, double a_contentScaleY = 1.0);

    //! Destructor of cViewport
    virtual ~cViewport();


    //-----------------------------------------------------------------------
    // PUBLIC METHODS - GENERAL:
    //-----------------------------------------------------------------------

public:

    //! This method returns a pointer to the current camera.
    cCamera* getCamera() { return (m_camera); }

    //! This method sets the camera.
    void setCamera(cCamera* a_camera);


    //-----------------------------------------------------------------------
    // PUBLIC METHODS - MOUSE SELECTION:
    //-----------------------------------------------------------------------

public:

    //! This method queries whether the specified position is 'pointing at' any objects in the world.
    virtual bool selectWorld(const int a_windowPosX, const int a_windowPosY,
        const int a_windowWidth, const int a_windowHeight,
        cCollisionRecorder& a_collisionRecorder,
        cCollisionSettings& a_collisionSettings);

    //! This method queries whether the specified position is 'pointing at' any widget on the front layer.
    virtual bool selectFrontLayer(const int a_windowPosX, const int a_windowPosY,
        const int a_windowWidth, const int a_windowHeight,
        cCollisionRecorder& a_collisionRecorder,
        cCollisionSettings& a_collisionSettings);

    //! This method queries whether the specified position is 'pointing at' any widget on the back layer.
    virtual bool selectBackLayer(const int a_windowPosX, const int a_windowPosY,
        const int a_windowWidth, const int a_windowHeight,
        cCollisionRecorder& a_collisionRecorder,
        cCollisionSettings& a_collisionSettings);

    //! This method queries whether the specified position is 'pointing at' any widget on selected layers.
    virtual bool selectLayers(const int a_windowPosX, const int a_windowPosY,
        const int a_windowWidth, const int a_windowHeight,
        cCollisionRecorder& a_collisionRecorder,
        cCollisionSettings& a_collisionSettings,
        bool a_checkFrontLayer = true,
        bool a_checkBackLayer = true);


    //-----------------------------------------------------------------------
    // PUBLIC METHODS - RENDERING AND IMAGING:
    //-----------------------------------------------------------------------

public:

    //! This method renders the the viewport view in OpenGL
    virtual void renderView(const int a_windowWidth,
        const int a_windowHeight,
        const cEyeMode a_eyeMode = C_STEREO_LEFT_EYE,
        const bool a_defaultBuffer = true);

    //! This method copies the output image data to an image structure.
    void copyImageBuffer(cImagePtr a_image);

    //! This method returns the width of the current window display in pixels.
    int getDisplayWidth();

    //! This method returns the height of the current window display in pixels.
    int getDisplayHeight();

    //! This method sets the content scale factor along the width axis.
    void setContentScaleWidth(double a_contentScaleWidth = 1.0) { setContentScale(a_contentScaleWidth, m_contentScaleHeight); };

    //! This method sets the content scale factor along the height axis.
    void setContentScaleHeight(double a_contentScaleHeight = 1.0) { setContentScale(m_contentScaleWidth, a_contentScaleHeight); };

    //! This method sets the content scale factors along the width and height axes.
    void setContentScale(double a_contentScaleWidth = 1.0, double a_contentScaleHeight = 1.0);

    //! This method returns the content scale factor along the width axis.
    double getContentScaleWidth() { return m_contentScaleWidth; }

    //! This method returns the content scale factor along the height axis.
    double getContentScaleHeight() { return m_contentScaleHeight; }

    //! This method resets textures and display lists for the world associated with this camera.
    void updateGPU();


    //-----------------------------------------------------------------------
    // PROTECTED MEMBERS:
    //-----------------------------------------------------------------------

protected:

    //! Camera.
    cCamera *m_camera;

    //! Camera.
    cCamera* m_cameraViewport;

    //! Content scale factor along width axis.
    double m_contentScaleWidth;

    //! Content scale factor along height axis.
    double m_contentScaleHeight;

    //! View panel
    cViewPanel* m_viewPanel;

    //! Display buffer
    cFrameBufferPtr m_displayBuffer;

    //! Display buffer width in pixels
    double m_displayWidth;

    //! Display buffer height in pixels
    double m_displayHeight;
};

//------------------------------------------------------------------------------
} // namespace chai3d
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
#endif
//------------------------------------------------------------------------------
