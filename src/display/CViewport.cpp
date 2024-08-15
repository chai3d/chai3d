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
#include "display/CViewport.h"
using namespace std;
//------------------------------------------------------------------------------
#include "world/CWorld.h"
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
namespace chai3d {
//------------------------------------------------------------------------------

//==============================================================================
/*!
    Constructor of cViewport.

    \param  a_camera  Camera used to render the scene.
    \param  a_contentScaleWidth  Content scale factor along horizontal axis.
    \param  a_contentScaleHeight Content scale factor along vertical axis.
*/
//==============================================================================
cViewport::cViewport(cCamera* a_camera, double a_contentScaleWidth, double a_contentScaleHeight)
{
    m_displayWidth = 0;
    m_displayHeight = 0;

    m_camera = a_camera;
    m_contentScaleWidth = a_contentScaleWidth;
    m_contentScaleHeight = a_contentScaleHeight;

    m_displayBuffer = cFrameBuffer::create();
    m_displayBuffer->setup(m_camera);

    m_cameraViewport = new cCamera(nullptr);

    // create and setup view panel 1
    m_viewPanel = new cViewPanel(m_displayBuffer);
    m_cameraViewport->m_frontLayer->addChild(m_viewPanel);
}


//==============================================================================
/*!
    Destructor of cViewport.
*/
//==============================================================================
cViewport::~cViewport()
{
    delete m_cameraViewport;
    delete m_viewPanel;
}


//==============================================================================
/*!
    This method sets the camera.
*/
//==============================================================================
void cViewport::setCamera(cCamera* a_camera)
{ 
    m_camera = a_camera;
    m_displayBuffer->setCamera(m_camera);

    // create and setup view panel 1
    m_viewPanel = new cViewPanel(m_displayBuffer);
    m_cameraViewport->m_frontLayer->addChild(m_viewPanel);
}


//==============================================================================
/*!
    This method sets the content scale factors along the width and height axes.

    \param  a_contentScaleWidth   Content scale factor along width axis
    \param  a_contentScaleHeight  Content scale factor along height axis
*/
//==============================================================================
void cViewport::setContentScale(double a_contentScaleWidth, double a_contentScaleHeight)
{
    m_contentScaleWidth = cMax(0.01, a_contentScaleWidth);
    m_contentScaleHeight = cMax(0.01, a_contentScaleHeight);
}


//==============================================================================
/*!
    This method checks for collision detection between an x-y position 
    (typically a mouse click) and an object in the scene.

    \param  a_windowPosX         X coordinate position of mouse click.
    \param  a_windowPosY         Y coordinate position of mouse click.
    \param  a_windowWidth        Width of window display (pixels)
    \param  a_windowHeight       Height of window display (pixels)
    \param  a_collisionRecorder  Recorder used to store all collisions between mouse and objects
    \param  a_collisionSettings  Settings related to collision detection

    \return __true__ if an object has been hit, __false__ otherwise.
*/
//==============================================================================
bool cViewport::selectWorld(const int a_windowPosX,
                          const int a_windowPosY,
                          const int a_windowWidth,
                          const int a_windowHeight,
                          cCollisionRecorder& a_collisionRecorder,
                          cCollisionSettings& a_collisionSettings)
{
    bool result = false;

    if (m_camera != nullptr)
    {
        result = m_camera->selectWorld(a_windowPosX,
            a_windowPosY,
            a_windowWidth,
            a_windowHeight,
            a_collisionRecorder,
            a_collisionSettings);
    }

    return result;
}


//==============================================================================
/*!
    This method checks for collision detection between an x-y position 
    (typically a mouse click) and a widget on the front layer. The (0,0) 
    coordinate is located at the bottom left pixel on the screen.

    \param  a_windowPosX         X coordinate position of mouse click.
    \param  a_windowPosY         Y coordinate position of mouse click.
    \param  a_windowWidth        Width of window display (pixels)
    \param  a_windowHeight       Height of window display (pixels)
    \param  a_collisionRecorder  Recorder used to store all collisions between mouse and objects
    \param  a_collisionSettings  Settings related to collision detection

    \return __true__ if an object has been hit, otherwise __false__.
*/
//==============================================================================
bool cViewport::selectFrontLayer(const int a_windowPosX, const int a_windowPosY,
    const int a_windowWidth, const int a_windowHeight,
    cCollisionRecorder& a_collisionRecorder,
    cCollisionSettings& a_collisionSettings)
{
    // sanity check
    if ((a_windowWidth == 0) || (a_windowHeight == 0))
    {
        return false;
    }

    bool result = false;

    if (m_camera != nullptr)
    {
        double scaleW = m_displayWidth / a_windowWidth;
        double scaleH = m_displayWidth / a_windowWidth;

        result = m_camera->selectFrontLayer(scaleW *a_windowPosX,
            scaleH * a_windowPosY,
            scaleW * a_windowWidth,
            scaleH * a_windowHeight,
            a_collisionRecorder,
            a_collisionSettings);
    }

    return result;
}


//==============================================================================
/*!
    This method checks for collision detection between an x-y position 
    (typically a mouse click) and a widget on the back layer. 
    The (0,0) coordinate is located at the bottom left pixel on the screen.

    \param  a_windowPosX         X coordinate position of mouse click.
    \param  a_windowPosY         Y coordinate position of mouse click.
    \param  a_windowWidth        Width of window display (pixels)
    \param  a_windowHeight       Height of window display (pixels)
    \param  a_collisionRecorder  Recorder used to store all collisions between mouse and objects
    \param  a_collisionSettings  Settings related to collision detection

    \return __true__ if an object has been hit, otherwise __false__.
*/
//==============================================================================
bool cViewport::selectBackLayer(const int a_windowPosX, const int a_windowPosY,
    const int a_windowWidth, const int a_windowHeight,
    cCollisionRecorder& a_collisionRecorder,
    cCollisionSettings& a_collisionSettings)
{
    bool result = false;

    if (m_camera != nullptr)
    {
        result = m_camera->selectBackLayer(a_windowPosX / m_contentScaleWidth,
            a_windowPosY / m_contentScaleHeight,
            a_windowWidth / m_contentScaleWidth,
            a_windowHeight / m_contentScaleHeight,
            a_collisionRecorder,
            a_collisionSettings);
    }

    return result;
}


//==============================================================================
/*!
    This method checks for collision detection between an x-y position 
    (typically a mouse click) and a widget on the front layer. 
    The (0,0) coordinate is located at the bottom left pixel on the screen.

    \param  a_windowPosX         X coordinate position of mouse click.
    \param  a_windowPosY         Y coordinate position of mouse click.
    \param  a_windowWidth        Width of window display (pixels)
    \param  a_windowHeight       Height of window display (pixels)
    \param  a_collisionRecorder  Recorder used to store all collisions between mouse and objects
    \param  a_collisionSettings  Settings related to collision detection
    \param  a_checkFrontLayer    If __true__, select front layer.
    \param  a_checkBackLayer     If __true__, select back layer.

    \return __true__ if an object has been hit, otherwise __false__.
*/
//==============================================================================
bool cViewport::selectLayers(const int a_windowPosX, const int a_windowPosY,
    const int a_windowWidth, const int a_windowHeight,
    cCollisionRecorder& a_collisionRecorder,
    cCollisionSettings& a_collisionSettings,
    bool a_checkFrontLayer,
    bool a_checkBackLayer)
{
    bool result = false;

    if (m_camera != nullptr)
    {
        result = m_camera->selectLayers(a_windowPosX / m_contentScaleWidth,
            a_windowPosY / m_contentScaleHeight,
            a_windowWidth / m_contentScaleWidth,
            a_windowHeight / m_contentScaleHeight,
            a_collisionRecorder,
            a_collisionSettings);
    }

    return result;
}


//==============================================================================
/*!
    This method renders the scene viewed by the camera.

    \param  a_windowWidth     Width of viewport.
    \param  a_windowHeight    Height of viewport.
    \param  a_eyeMode         When using stereo mode C_STEREO_PASSIVE_DUAL_DISPLAY, 
                              specifies which eye view to render.
    \param  a_defaultBuffer   If __true__ then the scene is rendered in the default
                              OpenGL buffer. If __false_ then the scene is rendered 
                              in a framebuffer (cFrameBuffer) that will have been 
                              previously setup.
*/
//==============================================================================
void cViewport::renderView(const int a_windowWidth,
                         const int a_windowHeight,
                         const cEyeMode a_eyeMode,
                         const bool a_defaultBuffer)
{
    // update view panel
    m_viewPanel->setSize(a_windowWidth, a_windowHeight);

    // update framebuffer size
    m_displayWidth = a_windowWidth / m_contentScaleWidth;
    m_displayHeight = a_windowHeight / m_contentScaleHeight;
    m_displayBuffer->setSize(m_displayWidth, m_displayHeight);

    // render framebuffer
    m_displayBuffer->renderView();

    // render viewport
    m_cameraViewport->renderView(a_windowWidth, a_windowHeight, a_eyeMode, a_defaultBuffer);
}


//==============================================================================
/*!
    This method copies the OpenGL image buffer to a cImage class structure.

    \param  a_image  Destination image.
*/
//==============================================================================
void cViewport::copyImageBuffer(cImagePtr a_image)
{
    if (m_camera != nullptr)
    {
        m_camera->copyImageBuffer(a_image);
    }
}


//==============================================================================
/*!
    This method returns the width of the current window display in pixels.
*/
//==============================================================================
int cViewport::getDisplayWidth()
{
    return m_displayWidth;
}


//==============================================================================
/*!
    This method returns the height of the current window display in pixels.
*/
//==============================================================================
int cViewport::getDisplayHeight()
{
    return m_displayHeight;
}


//==============================================================================
/*!
    This method updates all display lists and textures to the GPU.
*/
//==============================================================================
void cViewport::updateGPU()
{
    if (m_camera != nullptr)
    {
        m_camera->updateGPU();
    }
}


//------------------------------------------------------------------------------
} // namespace chai3d
//------------------------------------------------------------------------------
