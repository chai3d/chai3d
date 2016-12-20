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
#include "CODEWorld.h"
//---------------------------------------------------------------------------
using namespace chai3d;
using namespace std;
//---------------------------------------------------------------------------
//! Maximum number of contact points per body.
#define MAX_CONTACTS_PER_BODY 16
//---------------------------------------------------------------------------


//==========================================================================
/*!
    Constructor of cODEWorld.

    \param  a_parentWorld  Pointer to parent CHAI3D world.
*/
//===========================================================================
cODEWorld::cODEWorld(cWorld* a_parentWorld)
{
    // init ODE
    dInitODE();

    // set parent world
    m_parentWorld = a_parentWorld;

    // reset simulation time.
    m_simulationTime = 0.0;

    // create ODE world
    m_ode_world = dWorldCreate();

    // create ODE space
    m_ode_space = dHashSpaceCreate(0);

    // setup callback to handle collision
    dSpaceCollide(m_ode_space, this, &(cODEWorld::nearCallback));

    // create ODE contact group
    m_ode_contactgroup = dJointGroupCreate(0);

    // set some default damping parameters
    dWorldSetLinearDamping(m_ode_world, 0.00001);
    dWorldSetAngularDamping(m_ode_world, 0.0015);
    dWorldSetMaxAngularSpeed(m_ode_world, 200);
}


//===========================================================================
/*!
    Destructor of cODEWorld.
*/
//===========================================================================
cODEWorld::~cODEWorld()
{
    // clear all bodies
    m_bodies.clear();

    // cleanup ODE
    dJointGroupDestroy(m_ode_contactgroup);
    dSpaceDestroy(m_ode_space);
    dWorldDestroy(m_ode_world);
}


//===========================================================================
/*!
    This methods defines a gravity field passed as argument.

    \param  a_gravity  Gravity field vector.
*/
//===========================================================================
void cODEWorld::setGravity(cVector3d a_gravity)
{
    // update ode
    dWorldSetGravity (m_ode_world, a_gravity.x(), a_gravity.y(), a_gravity.z());
}


//===========================================================================
/*!
    This methods returns the current gravity field vector.

    \return Current gravity field vector.
*/
//===========================================================================
cVector3d cODEWorld::getGravity()
{
    // temp
    cVector3d result;
    dVector3 gravity;

    // return result
    dWorldGetGravity (m_ode_world, gravity);
    result.set(gravity[0], gravity[1], gravity[2]);
    return (result);
}


//===========================================================================
/*!
    This methods renders the world graphically using OpenGL.

    \param  a_options  Rendering options.
*/
//===========================================================================
void cODEWorld::render(cRenderOptions& a_options)
{
    list<cODEGenericBody*>::iterator i;

    // render all dynamic ODE bodies
    for(i = m_bodies.begin(); i != m_bodies.end(); ++i)
    {
        cODEGenericBody* nextItem = *i;
        nextItem->renderSceneGraph(a_options);
    }
}


//===========================================================================
/*!
    This methods computes globalPos and globalRot given the localPos and
    localRot of this object and its parents.

    If \a a_frameOnly is set to __false__, additional global positions such as
    vertex positions are computed (which can be time-consuming).

    \param  a_frameOnly  If __true__ then only the global frame is computed.
*/
//===========================================================================
void cODEWorld::updateGlobalPositions(const bool a_frameOnly)
{
    list<cODEGenericBody*>::iterator i;

    for(i = m_bodies.begin(); i != m_bodies.end(); ++i)
    {
        cODEGenericBody* nextItem = *i;
        nextItem->computeGlobalPositions(a_frameOnly,
                                         m_globalPos,
                                         m_globalRot);
    }
};


//===========================================================================
/*!
    This methods updates the simulation over a time interval passed as
    argument

    \param  a_interval  Time increment.
*/
//===========================================================================
void cODEWorld::updateDynamics(double a_interval)
{
    // sanity check
    if (a_interval <= 0) { return; }

    // update collision callback information
    dSpaceCollide (m_ode_space, 0, &(cODEWorld::nearCallback));

    // integrate simulation during an certain interval
    // dWorldStep (m_ode_world, a_interval);
    // dWorldStepFast1 (m_ode_world, a_interval, 5);
    dWorldQuickStep (m_ode_world, a_interval);

    // cleanup contacts from previous iteration
    dJointGroupEmpty(m_ode_contactgroup);

    // add time to overall simulation
    m_simulationTime = m_simulationTime + a_interval;

    // update CHAI3D positions for of all object
    updateBodyPositions();
}


//===========================================================================
/*!
    This methods updates the position and orientation from ODE models 
    to CHAI3D models.
*/
//===========================================================================
void cODEWorld::updateBodyPositions(void)
{
    list<cODEGenericBody*>::iterator i;

    for(i = m_bodies.begin(); i != m_bodies.end(); ++i)
    {
        cODEGenericBody* nextItem = *i;
        nextItem->updateBodyPosition();
    }
}


//===========================================================================
/*!
    This methods is an ODE callback for handling collision detection.

    \param  a_data     Not used here.
    \param  a_object1  Reference to ODE object 1.
    \param  a_object2  Reference to ODE object 2.
*/
//===========================================================================
void cODEWorld::nearCallback (void *a_data, dGeomID a_object1, dGeomID a_object2)
{
    // retrieve body ID for each object. This value is defined unless the object
    // is static.
    dBodyID b1 = dGeomGetBody(a_object1);
    dBodyID b2 = dGeomGetBody(a_object2);

    // exit without doing anything if the two bodies are connected by a joint
    if (b1 && b2 && dAreConnectedExcluding (b1,b2,dJointTypeContact)) return;

    dContact contact[MAX_CONTACTS_PER_BODY];
    int n = dCollide (a_object1, a_object2, MAX_CONTACTS_PER_BODY,&(contact[0].geom),sizeof(dContact));
    if (n > 0) 
    {
        for (int i=0; i<n; i++) 
        {
            // define default collision properties (this section could be extended to support some ODE material class!)
            contact[i].surface.slip1 = 0.7;
            contact[i].surface.slip2 = 0.7;
            contact[i].surface.mode = dContactSoftERP | dContactSoftCFM | dContactApprox1 | dContactSlip1 | dContactSlip2;
            contact[i].surface.mu = 50;
            contact[i].surface.soft_erp = 0.90;
            contact[i].surface.soft_cfm = 0.10;

            // get handles on each body
            cODEGenericBody* ode_body = NULL;
            if (b1 != NULL)
            {
                ode_body = (cODEGenericBody*)dBodyGetData (b1);
            }
            else if (b2 != NULL)
            {
                // if first object is static, use the second one. (both objects can not be static) 
                ode_body = (cODEGenericBody*)dBodyGetData (b2);
            }
                
            // create a joint following collision
            if (ode_body != NULL)
            {
                dJointID c = dJointCreateContact (ode_body->m_ODEWorld->m_ode_world,
                                                    ode_body->m_ODEWorld->m_ode_contactgroup,
                                                    &contact[i]);
                dJointAttach (c,
                            dGeomGetBody(contact[i].geom.g1),
                            dGeomGetBody(contact[i].geom.g2));
            }

        }
    }
}


//==============================================================================
/*!
    This method checks if the given line segment intersects any object
    located inside the virtual world.

    \param  a_segmentPointA  Initial point of segment.
    \param  a_segmentPointB  End point of segment.
    \param  a_recorder       Recorder which stores all collision events.
    \param  a_settings       Structure which contains some rules about how the
    collision detection should be performed.

    \return __true__ if a collision occurred, __false__ otherwise.
*/
//==============================================================================
bool cODEWorld::computeCollisionDetection(const cVector3d& a_segmentPointA,
                                          const cVector3d& a_segmentPointB,
                                          cCollisionRecorder& a_recorder,
                                          cCollisionSettings& a_settings)
{
    ///////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    ///////////////////////////////////////////////////////////////////////////

    // check if node is a ghost. If yes, then ignore call
    if (m_ghostEnabled) { return (false); }

    // temp variable
    bool hit = false;

    // get the transpose of the local rotation matrix
    cMatrix3d transLocalRot;
    m_localRot.transr(transLocalRot);

    // convert first endpoint of the segment into local coordinate frame
    cVector3d localSegmentPointA = a_segmentPointA;
    localSegmentPointA.sub(m_localPos);
    transLocalRot.mul(localSegmentPointA);

    // convert second endpoint of the segment into local coordinate frame
    cVector3d localSegmentPointB = a_segmentPointB;
    localSegmentPointB.sub(m_localPos);
    transLocalRot.mul(localSegmentPointB);


    ///////////////////////////////////////////////////////////////////////////
    // CHECK COLLISIONS
    ///////////////////////////////////////////////////////////////////////////

    // check each ODE body
    list<cODEGenericBody*>::iterator i;
    for(i = m_bodies.begin(); i != m_bodies.end(); ++i)
    {
        cODEGenericBody *nextItem = *i;
        bool hitBody = nextItem->computeCollisionDetection(localSegmentPointA, 
                                                           localSegmentPointB, 
                                                           a_recorder,
                                                           a_settings);

        hit = hit | hitBody;
    }


    ///////////////////////////////////////////////////////////////////////////
    // CHECK CHILDREN
    ///////////////////////////////////////////////////////////////////////////

    // check for collisions with all children of this object
    for (unsigned int i=0; i<m_children.size(); i++)
    {
        // call this child's collision detection function to see if it (or any
        // of its descendants) are intersected by the segment
        bool hitChild = m_children[i]->computeCollisionDetection(localSegmentPointA,
                                                                 localSegmentPointB,
                                                                 a_recorder,
                                                                 a_settings);

        hit = hit | hitChild;
    }


    ///////////////////////////////////////////////////////////////////////////
    // FINALIZE
    ///////////////////////////////////////////////////////////////////////////

    // return whether there was a collision between the segment and this world
    return (hit);
}
