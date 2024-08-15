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
    \author    Krzysztof Kondrak
    \author    Francois Conti
    \version   3.3.0
*/
//==============================================================================

//------------------------------------------------------------------------------
#ifndef COculusDeviceH
#define COculusDeviceH
//------------------------------------------------------------------------------
#include "display/CCamera.h"
#include "graphics/COpenGLHeaders.h"
#include "math/CTransform.h"
#include "system/CGlobals.h"
//------------------------------------------------------------------------------
#include "OVR_CAPI.h"
#include "OVR_CAPI_GL.h"
#include "Extras/OVR_Math.h"
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
namespace chai3d {
//------------------------------------------------------------------------------

//---------------------------------------------------------------------------
//! \defgroup   OCULUS   Oculus visualization device
//! \brief      Implements a CHAI3D module for rendering CHAI3D scenes
//!             in the Oculus device.
//---------------------------------------------------------------------------

//==============================================================================
/*!
    \file       COculusDevice.h
    \ingroup    OCULUS

    \brief
    Implements an interface to an Oculus device.
*/
//==============================================================================

//@{

//==============================================================================
/*!
    \brief
    This function converts an oculus OVR matrix to a cTransform.

    \details
    This function converts an oculus OVR matrix to a cTransform.

    \param  a_matrix  Input matrix to be converted.

    \return The converted matrix.
*/
//==============================================================================
inline cTransform cOculusConvertMatrix(OVR::Matrix4f a_matrix)
{
    cTransform result;
    result(0, 0) = a_matrix.M[0][0]; result(0, 1) = a_matrix.M[0][1]; result(0, 2) = a_matrix.M[0][2]; result(0, 3) = a_matrix.M[0][3];
    result(1, 0) = a_matrix.M[1][0]; result(1, 1) = a_matrix.M[1][1]; result(1, 2) = a_matrix.M[1][2]; result(1, 3) = a_matrix.M[1][3];
    result(2, 0) = a_matrix.M[2][0]; result(2, 1) = a_matrix.M[2][1]; result(2, 2) = a_matrix.M[2][2]; result(2, 3) = a_matrix.M[2][3];
    result(3, 0) = a_matrix.M[3][0]; result(3, 1) = a_matrix.M[3][1]; result(3, 2) = a_matrix.M[3][2]; result(3, 3) = a_matrix.M[3][3];
    return (result);
}

//@}

//==============================================================================
/*!
    \class      cOculusDevice

    \brief
    This class implements support for the Oculus Rift DK2.

    \details
    This class implements support for the Oculus Rift DK2. The implementation
    uses SDK 0.8.0 (beta)
*/
//==============================================================================
class cOculusDevice
{
    //--------------------------------------------------------------------------
    // CONSTRUCTOR & DESTRUCTOR:
    //--------------------------------------------------------------------------

public:

    //! Constructor of cOculusDevice.
    cOculusDevice();

    //! Destructor of cOculusDevice.
    ~cOculusDevice();


    //--------------------------------------------------------------------------
    // PUBLIC METHODS:
    //--------------------------------------------------------------------------

public:

    //! This method initializes the display
    bool initialize(cCamera* a_camera = NULL, bool a_useMirrorWindow = true, double a_mirrorDisplayScaleFactor = 1.0);

    //! This method sets a camera.
    void setCamera(cCamera* a_camera) { m_camera = a_camera;  }

    //! This method return a pointer to the camera.
    cCamera* getCamera() { return (m_camera); }

    //! This method enables or disables the rendering in a mirrored window.
    void setUseMirror(bool a_useMirrorWindow) { m_useMirrorWindow = a_useMirrorWindow; }

    //! This method returns __true__ if the mirrored window is rendered, __false__ otherwise.
    bool getUseMirror() { return (m_useMirrorWindow); }

    //! This method returns the resolution width of the display.
    int getResolutionWidth() { return (m_resolutionWidth); }

    //! This method returns the resolution height of display.
    int getResolutionHeight() { return (m_resolutionHeight); }

    //! This method returns the mirrored window display width.
    int getWindowWidth() { return (m_windowWidth); }

    //! This method returns the mirrored window display height.
    int getWindowHeight() { return (m_windowHeight); }

    //! This method enables or disables the use of MSAA.
    void setUseMSAA(bool a_useMSAA) { m_useMSAA = a_useMSAA; }

    //! This method returns __true__ if MSAA is enabled, __false__ otherwise.
    bool getUseMSAA() const { return (m_useMSAA); }

    //! This method renders the camera scene to the Oculus display.
    void render();

    //! This method recenters the position of user in the scene.
    void recenterPose() { ovr_RecenterTrackingOrigin(m_hmdSession); }


    //--------------------------------------------------------------------------
    // PROTECTED METHODS:
    //--------------------------------------------------------------------------

protected:

    bool initVR();
    bool initVRBuffers(int windowWidth, int windowHeight);
    bool initNonDistortMirror(int windowWidth, int windowHeight); // create non-distorted mirror if necessary (debug purposes)
    void destroyVR();
    const ovrSizei getResolution() const;
    void onRenderStart();
    const OVR::Matrix4f onEyeRender(int eyeIndex);
    void onEyeRender(int eyeIndex, cTransform& a_projectionMatrix, cTransform& a_modelViewMatrix);
    void onEyeRenderFinish(int eyeIndex);
    const OVR::Matrix4f getEyeMVPMatrix(int eyeIdx) const;
    void submitFrame();

    void blitMirror(ovrEyeType numEyes=ovrEye_Count, int offset = 0);   // regular OculusVR mirror view
    void onNonDistortMirrorStart();        // non-distorted mirror rendering start (debug purposes)
    void blitNonDistortMirror(int offset); // non-distorted mirror rendering (debug purposes)

    void renderTrackerFrustum();
    bool isDebugHMD() const { return (m_hmdDesc.AvailableHmdCaps & ovrHmdCap_DebugDevice) != 0; }
    bool isDK2() const { return m_hmdDesc.Type == ovrHmd_DK2; }
    void showPerfStats(ovrPerfHudMode statsMode);

    ovrSizei getEyeTextureSize(int a_index) { return (m_eyeBuffers[a_index]->m_eyeTextureSize); }

    //--------------------------------------------------------------------------
    // PROTECTED MEMBERS:
    //--------------------------------------------------------------------------

protected:

    //! CHAI3D camera.
    cCamera* m_camera;

    //! Width resolution of oculus display.
    int m_resolutionWidth;

    //! Height resolution of oculus display.
    int m_resolutionHeight;

    //! Width of mirrored display.
    int m_windowWidth;

    //! Height of mirrored display.
    int m_windowHeight;

    //! Flag which indicates if mirrored display should be rendered.
    bool m_useMirrorWindow;


    //--------------------------------------------------------------------------
    // PROTECTED MEMBERS:
    //--------------------------------------------------------------------------

protected:

    // Buffer structure used to store eye textures and framebuffers.
    // We create one instance for the left eye, one for the right eye.
    // Final rendering is done via blitting two separate frame buffers into one render target.
    struct OVRBuffer
    {  
        OVRBuffer(const ovrSession &session, int eyeIdx);

        void onRender();
        void onRenderFinish();
        void setupMSAA(); 
        void onRenderMSAA();
        void onRenderMSAAFinish();
        void destroy(const ovrSession &session);

        ovrSizei   m_eyeTextureSize;
        GLuint     m_eyeFbo;
        GLuint     m_eyeTexId;
        GLuint     m_depthBuffer;

        GLuint m_msaaEyeFbo;     // framebuffer for MSAA texture
        GLuint m_eyeTexMSAA;     // color texture for MSAA
        GLuint m_depthTexMSAA;   // depth texture for MSAA

        ovrTextureSwapChain m_swapTextureChain;
    };

    // Data and buffers used to render to HMD
    ovrSession        m_hmdSession;
    ovrHmdDesc        m_hmdDesc;
    ovrEyeRenderDesc  m_eyeRenderDesc[ovrEye_Count];
    ovrPosef          m_eyeRenderPose[ovrEye_Count];
    ovrPosef          m_hmdToEyeOffset[ovrEye_Count];
    OVRBuffer        *m_eyeBuffers[ovrEye_Count];

    OVR::Matrix4f     m_projectionMatrix[ovrEye_Count];
    OVR::Matrix4f     m_eyeViewOffset[ovrEye_Count];
    OVR::Matrix4f     m_eyeOrientation[ovrEye_Count];
    OVR::Matrix4f     m_eyePose[ovrEye_Count];

    // Frame timing data and tracking info
    double            m_frameTiming;
    ovrTrackingState  m_trackingState;

    // Mirror texture used to render HMD view to OpenGL window
    ovrMirrorTexture     m_mirrorTexture;
    ovrMirrorTextureDesc m_mirrorDesc;

    // Debug non-distorted mirror texture data
    GLuint            m_nonDistortTexture;
    GLuint            m_nonDistortDepthBuffer;
    GLuint            m_mirrorFBO;
    GLuint            m_nonDistortFBO;
    int               m_nonDistortViewPortWidth;
    int               m_nonDistortViewPortHeight;
    bool              m_useMSAA;
    long long         m_frameIndex;
    double            m_sensorSampleTime;
};

//------------------------------------------------------------------------------
} // namespace chai3d
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
#endif
//------------------------------------------------------------------------------
