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
    \author    Federico Barbagli
    \version   3.3.0
*/
//==============================================================================

//------------------------------------------------------------------------------
#include "tools/CToolGripper.h"
#include "graphics/CTriangleArray.h"
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
namespace chai3d {
//------------------------------------------------------------------------------

//==============================================================================
/*!
    Constructor of cToolGripper.

    \param  a_parentWorld  World in which the tool will operate.
*/
//==============================================================================
cToolGripper::cToolGripper(cWorld* a_parentWorld):cGenericTool(a_parentWorld)
{
    // default gripper workspace
    m_gripperWorkspaceScale = 1.0;

    // create a haptic point contact for the thumb
    m_hapticPointThumb = new cHapticPoint(this);
    m_hapticPoints.push_back(m_hapticPointThumb);

    // create a haptic point contact for the finger
    m_hapticPointFinger = new cHapticPoint(this);
    m_hapticPoints.push_back(m_hapticPointFinger);

    // add point elements as components
    addComponent(m_hapticPointThumb->m_sphereGoal);
    addComponent(m_hapticPointThumb->m_sphereProxy);
    addComponent(m_hapticPointFinger->m_sphereGoal);
    addComponent(m_hapticPointFinger->m_sphereProxy);

    // show proxy spheres only
    setShowContactPoints(true, false);
}


//==============================================================================
/*!
    Destructor of cToolGripper.
*/
//==============================================================================
cToolGripper::~cToolGripper()
{
    delete  m_hapticPointThumb;
    delete  m_hapticPointFinger;
}


//==============================================================================
/*!
    This method computes the interaction forces between the tool and all
    objects located inside the virtual world.
*/
//==============================================================================
void cToolGripper::computeInteractionForces()
{
    // convert the angle of the gripper into a position in device coordinates. 
    // this value is device dependent.
    double gripperRadius = 0.04;

    if (m_hapticDevice)
    {
        cHapticDeviceModel model = m_hapticDevice->m_specifications.m_model;

        switch (model)
        {
        case C_HAPTIC_DEVICE_OMEGA_7L:
            gripperRadius = 0.040;
            break;

        case C_HAPTIC_DEVICE_OMEGA_7R:
            gripperRadius = 0.040;
            break;

        case C_HAPTIC_DEVICE_SIGMA_7L:
            gripperRadius = 0.040;
            break;

        case C_HAPTIC_DEVICE_SIGMA_7R:
            gripperRadius = 0.040;
            break;

        case C_HAPTIC_DEVICE_LAMBDA_7L:
            gripperRadius = 0.080;
            break;

        case C_HAPTIC_DEVICE_LAMBDA_7R:
            gripperRadius = 0.080;
            break;

        default:
            break;
        }
    }

   // computer positions
    double gripperPositionFinger = gripperRadius * cSinRad(m_gripperAngle + cDegToRad(1.0));
    double gripperPositionThumb = gripperRadius * cSinRad(-m_gripperAngle + cDegToRad(-1.0));

    // compute new position of thumb and finger 
    cVector3d lineFingerThumb(0, 1, 0);
    cVector3d pFinger = m_gripperWorkspaceScale * m_workspaceScaleFactor * gripperPositionFinger * lineFingerThumb;
    cVector3d pThumb  = m_gripperWorkspaceScale * m_workspaceScaleFactor * gripperPositionThumb  * lineFingerThumb;

    cVector3d posFinger, posThumb;

    if ((!m_hapticDevice) || (m_hapticDevice->m_specifications.m_rightHand))
    {
        posFinger = m_deviceGlobalPos + cMul(m_deviceGlobalRot, (1.0 * pFinger));
        posThumb = m_deviceGlobalPos + cMul(m_deviceGlobalRot, (1.0 * pThumb));
    }
    else
    {
        posFinger = m_deviceGlobalPos + cMul(m_deviceGlobalRot, (-1.0 * pFinger));
        posThumb  = m_deviceGlobalPos + cMul(m_deviceGlobalRot, (-1.0 * pThumb));
    }

    // compute forces
    cVector3d forceThumb = m_hapticPointThumb->computeInteractionForces(posThumb, 
                                                                        m_deviceGlobalRot, 
                                                                        m_deviceGlobalLinVel, 
                                                                        m_deviceGlobalAngVel);

    cVector3d forceFinger = m_hapticPointFinger->computeInteractionForces(posFinger,
                                                                          m_deviceGlobalRot, 
                                                                          m_deviceGlobalLinVel, 
                                                                          m_deviceGlobalAngVel);

    // compute torques
    double scl = 0.0;
    double factor = m_gripperWorkspaceScale * m_workspaceScaleFactor;
    if (factor > 0.0)
    {
        scl = 1.0 / factor;
    }
    cVector3d torque = scl * cAdd(cCross(cSub(posThumb, m_deviceGlobalPos), forceThumb), cCross(cSub(posFinger, m_deviceGlobalPos), forceFinger));

    // compute gripper force
    double gripperForce = 0.0;

    if (m_hapticDevice &&
        ((m_hapticDevice->m_specifications.m_model == C_HAPTIC_DEVICE_OMEGA_7L) ||
         (m_hapticDevice->m_specifications.m_model == C_HAPTIC_DEVICE_SIGMA_7R) ||
         (m_hapticDevice->m_specifications.m_model == C_HAPTIC_DEVICE_SIGMA_7L)   ||
         (m_hapticDevice->m_specifications.m_model == C_HAPTIC_DEVICE_SIGMA_7R)   ||
         (m_hapticDevice->m_specifications.m_model == C_HAPTIC_DEVICE_LAMBDA_7L)   ||
         (m_hapticDevice->m_specifications.m_model == C_HAPTIC_DEVICE_LAMBDA_7R)))
    {
        cVector3d dir = posFinger - posThumb;
        if (dir.length() > 0.00001)
        {
            dir.normalize ();
            cVector3d force = cProject (forceFinger, dir);
            gripperForce = force.length();
            if (force.length() > 0.001)
            {
                double angle = cAngle(dir, force);
                if ((angle > C_PI/2.0) || (angle < -C_PI/2.0)) gripperForce = -gripperForce;
            }
        }
    }

    // finalize forces, torques and gripper force
    cVector3d globalForce = forceThumb + forceFinger;
    cVector3d globalTorque = torque;

    // gripper damping
    if (m_hapticDevice)
    {
        double gripperAngularVelocity = 0.0;
        m_hapticDevice->getGripperAngularVelocity(gripperAngularVelocity);
        double gripperDamping = -0.1 * m_hapticDevice->m_specifications.m_maxGripperAngularDamping * gripperAngularVelocity;
        gripperForce = gripperForce + gripperDamping;
    }

    // update computed forces to tool
    setDeviceGlobalForce(globalForce);
    setDeviceGlobalTorque(globalTorque);
    setGripperForce(gripperForce);
}


//==============================================================================
/*!
    This method renders the current tool using OpenGL.

    \param  a_options  Rendering options.
*/
//==============================================================================
void cToolGripper::render(cRenderOptions& a_options)
{
    ///////////////////////////////////////////////////////////////////////
    // render haptic points
    ///////////////////////////////////////////////////////////////////////
    int numContactPoint = (int)(m_hapticPoints.size());
    for (int i=0; i<numContactPoint; i++)
    {
        // get next haptic point
        cHapticPoint* nextContactPoint = m_hapticPoints[i];

        // render tool
        nextContactPoint->render(a_options);
    }

    ///////////////////////////////////////////////////////////////////////
    // render mesh image
    ///////////////////////////////////////////////////////////////////////
    if (m_image != NULL)
    {
        m_image->renderSceneGraph(a_options);
    }
}


//------------------------------------------------------------------------------
} // namespace chai3d
//------------------------------------------------------------------------------
