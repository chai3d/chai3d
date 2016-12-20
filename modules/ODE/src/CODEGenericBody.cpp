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
    \version   3.2.0 $Rev: 2097 $
*/
//===========================================================================

//---------------------------------------------------------------------------
#include "CODEGenericBody.h"
using namespace chai3d;
//---------------------------------------------------------------------------
#include "CODEWorld.h"
//---------------------------------------------------------------------------
#ifdef MACOSX
#include "OpenGL/glu.h"
#else
#include "GL/glu.h"
#endif
//---------------------------------------------------------------------------

//===========================================================================
/*!
    This method initializes the ODE model.

    \param  a_world  ODE world to which this new object belongs to.
*/
//===========================================================================
void cODEGenericBody::initialize(cODEWorld* a_world)
{
    // some default mass
    m_mass = 1.0;

    // store parent world
    m_ODEWorld = a_world;

    // no body geometry defined yet
    m_imageModel = new cGenericObject();

    // add body to world
    m_ODEWorld->m_bodies.push_back(this);

    // init ODE data
    m_ode_triMeshDataID = NULL;
    m_ode_body = NULL;

    m_prevTransform[0] = 1.0;
    m_prevTransform[1] = 0.0;
    m_prevTransform[2] = 0.0;
    m_prevTransform[3] = 0.0;
    m_prevTransform[4] = 0.0;
    m_prevTransform[5] = 1.0;
    m_prevTransform[6] = 0.0;
    m_prevTransform[7] = 0.0;
    m_prevTransform[8] = 0.0;
    m_prevTransform[9] = 0.0;
    m_prevTransform[10] = 1.0;
    m_prevTransform[11] = 0.0;
    m_prevTransform[12] = 0.0;
    m_prevTransform[13] = 0.0;
    m_prevTransform[14] = 0.0;
    m_prevTransform[15] = 1.0;

    // set a default color that shall be used to render the dynamic collision model
    m_colorDynamicCollisionModel.set(1.0, 1.0, 0.0);

    // hide dynamic collision model
    m_showDynamicCollisionModel = false;

    // set default values
    m_posOffsetDynColModel.zero();
    m_rotOffsetDynColModel.identity();
    m_paramDynColModel0 = 0.0;
    m_paramDynColModel1 = 0.0;
    m_paramDynColModel2 = 0.0;
}


//===========================================================================
/*!
    This method assigns a mass value to the current object.

    \param  a_mass  mass value.
*/
//===========================================================================
void cODEGenericBody::setMass(const double a_mass)
{
    // check if ODE body defined
    if (m_ode_body == NULL) { return; }

    // store value
    m_mass = a_mass;

    // adjust mass
    dMassAdjust(&m_ode_mass, a_mass);
    dBodySetMass(m_ode_body,&m_ode_mass);
}


//===========================================================================
/*!
    This method assigns a desired position to the object.

    \param  a_position  New desired position.
*/
//===========================================================================
void cODEGenericBody::setLocalPos(const chai3d::cVector3d &a_position)
{
    // check if body defined
    if (m_ode_body != NULL)
    {
        // store value
        m_localPos = a_position;

        // adjust position
        dBodySetPosition(m_ode_body, a_position.x(), a_position.y(), a_position.z());
    }
    else if (m_ode_geom != NULL)
    {
        // store value
        m_localPos = a_position;

        // adjust position
        dGeomSetPosition(m_ode_geom, a_position.x(), a_position.y(), a_position.z());
    }
}


//===========================================================================
/*!
    This method assigns a desired rotation to the object.

    \param  a_rotation  New desired orientation.
*/
//===========================================================================
void cODEGenericBody::setLocalRot(const cMatrix3d &a_rotation)
{
    // apply new rotation to ODE body
    dMatrix3 R;

    R[0]  = a_rotation(0,0);
    R[1]  = a_rotation(0,1);
    R[2]  = a_rotation(0,2);
    R[4]  = a_rotation(1,0);
    R[5]  = a_rotation(1,1);
    R[6]  = a_rotation(1,2);
    R[8]  = a_rotation(2,0);
    R[9]  = a_rotation(2,1);
    R[10] = a_rotation(2,2);

    // check if body defined
    if (m_ode_body != NULL)
    {
        // store new rotation matrix
        m_localRot = a_rotation;
        dBodySetRotation(m_ode_body, R);
    }
    else if (m_ode_geom != NULL)
    {
        // store new rotation matrix
        m_localRot = a_rotation;
        dGeomSetRotation(m_ode_geom, R);
    }
}


//===========================================================================
/*!
    This method applies an external force to the object at a given position.
    the position and force are expressed in global coordinates.

    \param  a_force  Force vector.
    \param  a_pos    Position (in global coordinates) where force is applied.
*/
//===========================================================================
void cODEGenericBody::addExternalForceAtPoint(const cVector3d& a_force, 
                                              const cVector3d& a_pos)
{
    if (m_ode_body != NULL)
    {
        dBodyAddForceAtPos(m_ode_body,
                        a_force.x(), a_force.y(), a_force.z(),
                        a_pos.x(), a_pos.y(), a_pos.z());
    }
}


//===========================================================================
/*!
    This method applies an external force at the center of mass of the object.

    \param  a_force  Force vector.
*/
//===========================================================================
void cODEGenericBody::addExternalForce(const cVector3d& a_force)
{
    dBodyAddForce(m_ode_body, a_force.x(), a_force.y(), a_force.z());
}


//===========================================================================
/*!
    This method applies an external torque at the center of mass of the object.

    \param  a_torque  Torque vector.
*/
//===========================================================================
void cODEGenericBody::addExternalTorque(const cVector3d& a_torque)
{
    dBodyAddTorque(m_ode_body, a_torque.x(), a_torque.y(), a_torque.z());
}


//===========================================================================
/*!
    This method updates the position and orientation data from the ODE 
    representation to the CHAI3D representation.
*/
//===========================================================================
void cODEGenericBody::updateBodyPosition(void)
{
    const double *odePosition;
    const double *odeRotation;

    // Retrieve position and orientation from ODE body.
    if (m_ode_body != NULL)
    {
        odePosition =  dBodyGetPosition(m_ode_body);
        odeRotation =  dBodyGetRotation(m_ode_body);
    }
    else
    {
        return;
    }

    // set new position
    m_localPos.set(odePosition[0],odePosition[1],odePosition[2]);

    // set new orientation
    m_localRot.set(odeRotation[0],odeRotation[1],odeRotation[2],
                   odeRotation[4],odeRotation[5],odeRotation[6],
                   odeRotation[8],odeRotation[9],odeRotation[10]);

    // store previous position if object is a mesh
    if (m_ode_triMeshDataID != NULL)
    {
        m_prevTransform[0] = odeRotation[0];
        m_prevTransform[1] = odeRotation[4];
        m_prevTransform[2] = odeRotation[8];
        m_prevTransform[3] = 0.0;
        m_prevTransform[4] = odeRotation[1];
        m_prevTransform[5] = odeRotation[5];
        m_prevTransform[6] = odeRotation[9];
        m_prevTransform[7] = 0.0;
        m_prevTransform[8] = odeRotation[2];
        m_prevTransform[9] = odeRotation[6];
        m_prevTransform[10] = odeRotation[10];
        m_prevTransform[11] = 0.0;
        m_prevTransform[12] = odePosition[0];
        m_prevTransform[13] = odePosition[1];
        m_prevTransform[14] = odePosition[2];
        m_prevTransform[15] = 1.0;

        dGeomTriMeshSetLastTransform(m_ode_geom, m_prevTransform);
    }

    // orthogonalize frame
    m_localRot.orthogonalize();
}


//===========================================================================
/*!
    This method checks if a given line segment intersects this object.

    \param  a_segmentPointA  Initial point of segment.
    \param  a_segmentPointB  End point of segment.
    \param  a_recorder       Recorder which stores all collision events.
    \param  a_settings       Structure which contains some rules about how the
                             collision detection should be performed.

    \return __true__ if collision occurred, __false__ otherwise.
*/
//===========================================================================
bool cODEGenericBody::computeCollisionDetection(const chai3d::cVector3d& a_segmentPointA,
                                                const chai3d::cVector3d& a_segmentPointB,
                                                chai3d::cCollisionRecorder& a_recorder,
                                                chai3d::cCollisionSettings& a_settings)
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
    // CHECK BODY IMAGE
    ///////////////////////////////////////////////////////////////////////////

    if (m_imageModel!=NULL)
    {
        hit = m_imageModel->computeCollisionDetection(localSegmentPointA,
                                                      localSegmentPointB,
                                                      a_recorder,
                                                      a_settings);
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

        // update if a hit occurred
        hit = hit | hitChild;
    }


    ///////////////////////////////////////////////////////////////////////////
    // FINALIZE
    ///////////////////////////////////////////////////////////////////////////

    // return result
    return (hit);
}


//===========================================================================
/*!
    This method assigns a generic object such as a mesh or a shape that is 
    used to model the geometry of this current ODE object.

    \param  a_imageModel  CHAI3D graphical image model.
*/
//===========================================================================
void cODEGenericBody::setImageModel(cGenericObject* a_imageModel)
{
    // check object
    if (a_imageModel == NULL) { return; }

    // store pointer to body
    m_imageModel = a_imageModel;

    // set external parent of body image to current ODE object.
    // rule applies to children which, in the case a mesh, belong
    // to the same object.
    m_imageModel->setOwner(this);

    // set parent
    m_imageModel->setParent(this);
}


//===========================================================================
/*!
    This method uses the bounding box of the geometric representation of 
    the object to generate a box that is used for collision detection.

    \param  a_staticObject  If __true__ then object is defined as static.
*/
//===========================================================================
void cODEGenericBody::createDynamicBoundingBox(const bool a_staticObject)
{
    // check if body image exists
    if (m_imageModel == NULL) { return; }

    // create ode dynamic body if object is non static
    if (!a_staticObject)
    {
        m_ode_body = dBodyCreate(m_ODEWorld->m_ode_world);

        // store pointer to current object
        dBodySetData (m_ode_body, this);
    }
    m_static = a_staticObject;

    // computing bounding box of geometry representation
    m_imageModel->computeBoundaryBox(true);

    // get size and center of box
    cVector3d center = m_imageModel->getBoundaryCenter();

    // compute dimensions
    cVector3d size = m_imageModel->getBoundaryMax() - m_imageModel->getBoundaryMin();

    // build box
    m_ode_geom = dCreateBox(m_ODEWorld->m_ode_space, size.x(), size.y() , size.z());

    // offset box
    dGeomSetPosition (m_ode_geom, center.x(), center.y(), center.z());

    if (!m_static)
    {
        // set inertia properties
        dMassSetBox(&m_ode_mass, 1.0, size.x(), size.y(), size.z());
        dMassAdjust(&m_ode_mass, m_mass);
        dBodySetMass(m_ode_body,&m_ode_mass);

        // attach body and geometry together
        dGeomSetBody(m_ode_geom, m_ode_body);
    }

    // store dynamic model type
    m_typeDynamicCollisionModel = ODE_MODEL_BOX;

    // store dynamic model parameters
    m_paramDynColModel0 = size.x();
    m_paramDynColModel1 = size.y();
    m_paramDynColModel2 = size.z();
    //m_posOffsetDynColModel;
    //m_rotOffsetDynColModel;
}


//===========================================================================
/*!
    This method creates an ODE dynamic sphere model that is used for collision 
    detection. It is possible to visualize the actual ODE physical model by
    calling \ref setShowDynamicCollisionModel() method.

    \param  a_radius        Radius of sphere.
    \param  a_staticObject  If __true__ then object is defined as static.
    \param  a_offsetPos     Position offset.
    \param  a_offsetRot     Rotation offset.
*/
//===========================================================================
void cODEGenericBody::createDynamicSphere(const double a_radius,
                                          const bool a_staticObject,
                                          const cVector3d& a_offsetPos,
                                          const cMatrix3d& a_offsetRot)
{
    // create ode dynamic body if object is non static
    if (!a_staticObject)
    {
        m_ode_body = dBodyCreate(m_ODEWorld->m_ode_world);

        // store pointer to current object
        dBodySetData (m_ode_body, this);
    }
    m_static = a_staticObject;

    // build sphere
    m_ode_geom = dCreateSphere(m_ODEWorld->m_ode_space, a_radius);

    // adjust position offset
    dGeomSetPosition (m_ode_geom, a_offsetPos.x(), a_offsetPos.y(), a_offsetPos.z());

    // adjust orientation offset
    dMatrix3 R;
    R[0]  = a_offsetRot(0,0);
    R[1]  = a_offsetRot(0,1);
    R[2]  = a_offsetRot(0,2);
    R[4]  = a_offsetRot(1,0);
    R[5]  = a_offsetRot(1,1);
    R[6]  = a_offsetRot(1,2);
    R[8]  = a_offsetRot(2,0);
    R[9]  = a_offsetRot(2,1);
    R[10] = a_offsetRot(2,2);
    dGeomSetRotation (m_ode_geom, R);

    // set inertia properties
    if (!m_static)
    {
        dMassSetSphere(&m_ode_mass, 1.0, a_radius);
        dMassAdjust(&m_ode_mass, m_mass);
        dBodySetMass(m_ode_body,&m_ode_mass);

        // attach body and geometry together
        dGeomSetBody(m_ode_geom, m_ode_body);
    }

    // store dynamic model type
    m_typeDynamicCollisionModel = ODE_MODEL_SPHERE;

    // store dynamic model parameters
    m_paramDynColModel0 = a_radius;
    m_paramDynColModel1 = 0.0;
    m_paramDynColModel2 = 0.0;
    m_posOffsetDynColModel = a_offsetPos;
    m_rotOffsetDynColModel = a_offsetRot;
}


//===========================================================================
/*!
    This method creates an ODE dynamic box model. 
    It is possible to visualize the actual ODE physical model by
    calling \ref setShowDynamicCollisionModel() method.

    \param  a_lengthX       Size of box along the __x__ axis.
    \param  a_lengthY       Size of box along the __y__ axis.
    \param  a_lengthZ       Size of box along the __z__ axis.
    \param  a_staticObject  If __true__ then object is defined as static.
    \param  a_offsetPos     Position offset.
    \param  a_offsetRot     Rotation offset.
*/
//===========================================================================
void cODEGenericBody::createDynamicBox(const double a_lengthX,
                                       const double a_lengthY,
                                       const double a_lengthZ,
                                       const bool a_staticObject,
                                       const cVector3d& a_offsetPos,
                                       const cMatrix3d& a_offsetRot)
{
    // create ode dynamic body if object is non static
    if (!a_staticObject)
    {
        m_ode_body = dBodyCreate(m_ODEWorld->m_ode_world);

        // store pointer to current object
        dBodySetData (m_ode_body, this);
    }
    m_static = a_staticObject;

    // build box
    m_ode_geom = dCreateBox(m_ODEWorld->m_ode_space, a_lengthX, a_lengthY, a_lengthZ);

    // adjust position offset
    dGeomSetPosition (m_ode_geom, a_offsetPos.x(), a_offsetPos.y(), a_offsetPos.z());

    // adjust orientation offset
    dMatrix3 R;
    R[0]  = a_offsetRot(0,0);
    R[1]  = a_offsetRot(0,1);
    R[2]  = a_offsetRot(0,2);
    R[4]  = a_offsetRot(1,0);
    R[5]  = a_offsetRot(1,1);
    R[6]  = a_offsetRot(1,2);
    R[8]  = a_offsetRot(2,0);
    R[9]  = a_offsetRot(2,1);
    R[10] = a_offsetRot(2,2);
    dGeomSetRotation (m_ode_geom, R);

    // set inertia properties
    if (!m_static)
    {
        dMassSetBox(&m_ode_mass, 1.0, a_lengthX, a_lengthY, a_lengthZ);
        dMassAdjust(&m_ode_mass, m_mass);
        dBodySetMass(m_ode_body,&m_ode_mass);

        // attach body and geometry together
        dGeomSetBody(m_ode_geom, m_ode_body);
    }

    // store dynamic model type
    m_typeDynamicCollisionModel = ODE_MODEL_BOX;

    // store dynamic model parameters
    m_paramDynColModel0 = a_lengthX;
    m_paramDynColModel1 = a_lengthY;
    m_paramDynColModel2 = a_lengthZ;
    m_posOffsetDynColModel = a_offsetPos;
    m_rotOffsetDynColModel = a_offsetRot;
}


//===========================================================================
/*!
    This method creates an ODE dynamic cylinder model (capsule).
    It is possible to visualize the actual ODE physical model by
    calling \ref setShowDynamicCollisionModel() method.

    \param  a_radius        Radius of capsule.
    \param  a_length        Length of capsule.
    \param  a_staticObject  If __true__ then object is defined as static.
    \param  a_offsetPos     Position offset.
    \param  a_offsetRot     Rotation offset.
*/
//===========================================================================
void cODEGenericBody::createDynamicCapsule(const double a_radius,
                                           const double a_length,
                                           const bool a_staticObject,
                                           const cVector3d& a_offsetPos,
                                           const cMatrix3d& a_offsetRot)
{
    // create ode dynamic body if object is non static
    if (!a_staticObject)
    {
        m_ode_body = dBodyCreate(m_ODEWorld->m_ode_world);

        // store pointer to current object
        dBodySetData (m_ode_body, this);
    }
    m_static = a_staticObject;

    // build box
    m_ode_geom = dCreateCapsule(m_ODEWorld->m_ode_space, a_radius, a_length);

    // adjust position offset
    dGeomSetPosition (m_ode_geom, a_offsetPos.x(), a_offsetPos.y(), a_offsetPos.z());

    // adjust orientation offset
    dMatrix3 R;
    R[0]  = a_offsetRot(0,0);
    R[1]  = a_offsetRot(0,1);
    R[2]  = a_offsetRot(0,2);
    R[4]  = a_offsetRot(1,0);
    R[5]  = a_offsetRot(1,1);
    R[6]  = a_offsetRot(1,2);
    R[8]  = a_offsetRot(2,0);
    R[9]  = a_offsetRot(2,1);
    R[10] = a_offsetRot(2,2);
    dGeomSetRotation (m_ode_geom, R);

    // set inertia properties
    if (!m_static)
    {
        dMassSetCylinder (&m_ode_mass, 1.0, 3, a_radius, a_length);  // 3 = x-axis direction
        dMassAdjust(&m_ode_mass, m_mass);
        dBodySetMass(m_ode_body,&m_ode_mass);

        // attach body and geometry together
        dGeomSetBody(m_ode_geom, m_ode_body);
    }

    // store dynamic model type
    m_typeDynamicCollisionModel = ODE_MODEL_CYLINDER;

    // store dynamic model parameters
    m_paramDynColModel0 = a_radius;
    m_paramDynColModel1 = a_length;
    m_paramDynColModel2 = 0.0;
    m_posOffsetDynColModel = a_offsetPos;
    m_rotOffsetDynColModel = a_offsetRot;
}


//===========================================================================
/*!
    This method Create an ODE dynamic cylinder model.
    It is possible to visualize the actual ODE physical model by
    calling \ref setShowDynamicCollisionModel() method.

    \param  a_radius        Radius of capsule.
    \param  a_length        Length of capsule.
    \param  a_staticObject  If __true__ then object is defined as static.
    \param  a_offsetPos     Position offset.
    \param  a_offsetRot     Rotation offset.
*/
//===========================================================================
void cODEGenericBody::createDynamicCylinder(const double a_radius,
                                            const double a_length,
                                            const bool a_staticObject,
                                            const cVector3d& a_offsetPos,
                                            const cMatrix3d& a_offsetRot)
{
    // create ode dynamic body if object is non static
    if (!a_staticObject)
    {
        m_ode_body = dBodyCreate(m_ODEWorld->m_ode_world);

        // store pointer to current object
        dBodySetData (m_ode_body, this);
    }
    m_static = a_staticObject;

    // build box
    m_ode_geom = dCreateCylinder(m_ODEWorld->m_ode_space, a_radius, a_length);

    // adjust position offset
    dGeomSetPosition (m_ode_geom, a_offsetPos.x(), a_offsetPos.y(), a_offsetPos.z());

    // adjust orientation offset
    dMatrix3 R;
    R[0]  = a_offsetRot(0,0);
    R[1]  = a_offsetRot(0,1);
    R[2]  = a_offsetRot(0,2);
    R[4]  = a_offsetRot(1,0);
    R[5]  = a_offsetRot(1,1);
    R[6]  = a_offsetRot(1,2);
    R[8]  = a_offsetRot(2,0);
    R[9]  = a_offsetRot(2,1);
    R[10] = a_offsetRot(2,2);
    dGeomSetRotation (m_ode_geom, R);

    // set inertia properties
    if (!m_static)
    {
        dMassSetCylinder (&m_ode_mass, 1.0, 3, a_radius, a_length);  // 3 = x-axis direction
        dMassAdjust(&m_ode_mass, m_mass);
        dBodySetMass(m_ode_body,&m_ode_mass);

        // attach body and geometry together
        dGeomSetBody(m_ode_geom, m_ode_body);
    }

    // store dynamic model type
    m_typeDynamicCollisionModel = ODE_MODEL_CYLINDER;

    // store dynamic model parameters
    m_paramDynColModel0 = a_radius;
    m_paramDynColModel1 = a_length;
    m_paramDynColModel2 = 0.0;
    m_posOffsetDynColModel = a_offsetPos;
    m_rotOffsetDynColModel = a_offsetRot;
}


//===========================================================================
/*!
    This method creates an ODE static model of a plane.
    It is possible to visualize the actual ODE physical model by
    calling \ref setShowDynamicCollisionModel() method.

    \param  a_position  Position of a point located on the plane.
    \param  a_normal    Surface normal of the plane.
*/
//===========================================================================
void cODEGenericBody::createStaticPlane(const cVector3d& a_position,
                                        const cVector3d& a_normal)
{
    // object is static by default
    m_static = true;

    // temp variables
    cVector3d normal = a_normal;
    cVector3d offset(0,0,0);

    // check normal
    if (normal.length() == 0) { return; }

    // compute parameters
    normal.normalize();
    double a = normal.x();
    double b = normal.y();
    double c = normal.z();

    offset = cProject(a_position, normal);
    double d = offset.length();
    if (d > 0)
    {
        if (cAngle(offset, normal) > cDegToRad(90))
        {
            d = -d;
        }
    }

    // build fixed plane
    m_ode_geom = dCreatePlane(m_ODEWorld->m_ode_space, a, b, c, d);

    // store dynamic model type
    m_typeDynamicCollisionModel = ODE_MODEL_PLANE;

    // store dynamic model parameters
    m_paramDynColModel0 = normal.x();
    m_paramDynColModel1 = normal.y();
    m_paramDynColModel2 = normal.z();
    m_posOffsetDynColModel = a_position;
    m_rotOffsetDynColModel.identity();
}


//===========================================================================
/*!
    This method creates an ODE dynamic model of a mesh.
    This model requires the body image to be a mesh and uses it triangles
    to build its physical model.

    \param  a_staticObject  If __true__ then object is defined as static.
    \param  a_offsetPos     Position offset.
    \param  a_offsetRot     Rotation offset.
*/
//===========================================================================
void cODEGenericBody::createDynamicMesh(const bool a_staticObject,
                                        const cVector3d& a_offsetPos,
                                        const cMatrix3d& a_offsetRot)
{
    // create ode dynamic body if object is non static
    if (!a_staticObject)
    {
        m_ode_body = dBodyCreate(m_ODEWorld->m_ode_world);

        // store pointer to current object
        dBodySetData (m_ode_body, this);
    }
    m_static = a_staticObject;

    // make sure body image has been defined
    if (m_imageModel == NULL) { return; }

    // check if body image is a mesh or multimesh
    cMesh* mesh = dynamic_cast<cMesh*>(m_imageModel);
    cMultiMesh* multiMesh = dynamic_cast<cMultiMesh*>(m_imageModel);
    if ((mesh == NULL) && (multiMesh == NULL)) { return; }

    // store dynamic model type
    m_typeDynamicCollisionModel = ODE_MODEL_TRIMESH;

    // store dynamic model parameters
    m_paramDynColModel0 = 0.0;
    m_paramDynColModel1 = 0.0;
    m_paramDynColModel2 = 0.0;
    m_posOffsetDynColModel = a_offsetPos;
    m_rotOffsetDynColModel = a_offsetRot;

    // declare variables
    int numTriangles;
    int vertexCount;
    int indexCount;
    int nTriangles;

    // create a table which lists all vertices and triangles
    // these vertices must be of type float and not double!
    // even if we are using the double precision compiled version of ODE !
    if (mesh != NULL)
    {
        numTriangles = mesh->getNumTriangles();

        vertexCount = mesh->getNumVertices();
        m_vertices = new float[3 * vertexCount];

        indexCount = 3 * numTriangles;
        m_vertexIndices = new int[indexCount];

        // build table of ODE vertices and vertex indices recursivly
        nTriangles = buildMeshTable(mesh);
    }
    else
    {
        numTriangles = multiMesh->getNumTriangles();

        vertexCount = multiMesh->getNumVertices();
        m_vertices = new float[3 * vertexCount];

        indexCount = 3 * numTriangles;
        m_vertexIndices = new int[indexCount];

        // build table of ODE vertices and vertex indices recursivly
        nTriangles = buildMeshTable(multiMesh);
    }

    // build box
    m_ode_triMeshDataID = dGeomTriMeshDataCreate();
    dGeomTriMeshDataBuildSingle(m_ode_triMeshDataID,
                                m_vertices,          // vertex positions
                                3 * sizeof(float),   // size of vertex
                                vertexCount,         // number of vertices
                                m_vertexIndices,     // triangle indices
                                3 * nTriangles,      // number of triangles
                                3 * sizeof(int));

    m_ode_geom = dCreateTriMesh(m_ODEWorld->m_ode_space, m_ode_triMeshDataID, 0, 0, 0);

    // remember the mesh's dTriMeshDataID on its userdata for convenience.
    dGeomSetData(m_ode_geom, m_ode_triMeshDataID);

    // computing bounding box of geometry representation
    m_imageModel->computeBoundaryBox(true);

    // compute dimensions
    cVector3d size = m_imageModel->getBoundaryMax() -
                     m_imageModel->getBoundaryMin();

    // set inertia properties
    if (!m_static)
    {
        dMassSetBox(&m_ode_mass, 1.0, size.x(), size.y(), size.z());
        dMassAdjust(&m_ode_mass, m_mass);
        dBodySetMass(m_ode_body,&m_ode_mass);

        dGeomSetBody(m_ode_geom, m_ode_body);
    }

    // adjust position offset
    dGeomSetPosition (m_ode_geom, a_offsetPos.x(), a_offsetPos.y(), a_offsetPos.z());

    // adjust orientation offset
    dMatrix3 R;
    R[0]  = a_offsetRot(0,0);
    R[1]  = a_offsetRot(0,1);
    R[2]  = a_offsetRot(0,2);
    R[4]  = a_offsetRot(1,0);
    R[5]  = a_offsetRot(1,1);
    R[6]  = a_offsetRot(1,2);
    R[8]  = a_offsetRot(2,0);
    R[9]  = a_offsetRot(2,1);
    R[10] = a_offsetRot(2,2);
    dGeomSetRotation (m_ode_geom, R);
}


//===========================================================================
/*!
    This method creates an ODE list of vertices and vertex indices from a 
    multimesh object.

    \param  a_multiMesh  Multimesh object.
*/
//===========================================================================
int cODEGenericBody::buildMeshTable(chai3d::cMultiMesh* a_multiMesh)
{
    // temp variables
    int nTriangles = 0;
    int vertexOffset = 0;

    // compute number of vertices
    int numVertices = a_multiMesh->getNumVertices();

    // store vertices
    for (int i=0; i<numVertices; i++)
    {
        cVector3d pos = a_multiMesh->getVertexPos(i);
        m_vertices[3*i+0] = (float)pos.x();
        m_vertices[3*i+1] = (float)pos.y();
        m_vertices[3*i+2] = (float)pos.z();
    }

    // store triangles for each mesh
    int numMeshes = (unsigned int)(a_multiMesh->m_meshes->size());
    int index = 0;
    for (int i=0; i<numMeshes; i++)
    {
        cMesh* nextMesh = a_multiMesh->m_meshes->at(i);
        int numTriangles = nextMesh->getNumTriangles();

        // store triangles
        for (int j=0; j<numTriangles; j++)
        {
            double area = nextMesh->m_triangles->computeArea(j);
            if (area > 0.0)
            {
                unsigned int vertex0 = nextMesh->m_triangles->getVertexIndex0(j) + vertexOffset;
                unsigned int vertex1 = nextMesh->m_triangles->getVertexIndex1(j) + vertexOffset;
                unsigned int vertex2 = nextMesh->m_triangles->getVertexIndex2(j) + vertexOffset;

                m_vertexIndices[index] = vertex0; index++;
                m_vertexIndices[index] = vertex1; index++;
                m_vertexIndices[index] = vertex2; index++;

                nTriangles++;
            }
        }

        vertexOffset = vertexOffset + nextMesh->getNumVertices();
    }

    return (nTriangles);
}


//===========================================================================
/*!
    This method creates an ODE list of vertices and vertex indices from a mesh
    and its children.

    \param  a_mesh  Mesh to be added to ODE vertex and triangle list.

    \return Number of triangle added to ODE list
*/
//===========================================================================
int cODEGenericBody::buildMeshTable(chai3d::cMesh* a_mesh)
{
    // temp variables
    int nTriangles = 0;

    // compute number of vertices and triangles for a_mesh
    unsigned int numTriangles = a_mesh->getNumTriangles();
    unsigned int numVertices = a_mesh->getNumVertices();

    // store vertices
    for (unsigned int i=0; i<numVertices; i++)
    {
        cVector3d pos = a_mesh->m_vertices->getLocalPos(i);
        m_vertices[3*i+0] = (float)pos.x();
        m_vertices[3*i+1] = (float)pos.y();
        m_vertices[3*i+2] = (float)pos.z();
    }

    // store triangles
    for (unsigned int i=0; i<numTriangles; i++)
    {
        double area = a_mesh->m_triangles->computeArea(i);
        if (area > 0)
        {
            unsigned int vertex0 = a_mesh->m_triangles->getVertexIndex0(i);
            unsigned int vertex1 = a_mesh->m_triangles->getVertexIndex1(i);
            unsigned int vertex2 = a_mesh->m_triangles->getVertexIndex2(i);
            m_vertexIndices[3*i+0] = vertex0;
            m_vertexIndices[3*i+1] = vertex1;
            m_vertexIndices[3*i+2] = vertex2;
            nTriangles++;
        }
    }

    return (nTriangles);
}


//===========================================================================
/*!
    This method enables the object from being updated within the dynamics
    simulation.
*/
//===========================================================================
void cODEGenericBody::enableDynamics()
{
    if (m_ode_body == NULL) { return; }
    dBodyEnable(m_ode_body);
    dGeomEnable(m_ode_geom);
}


//===========================================================================
/*!
    This method disables the object from being updated within the dynamics
    simulation.
*/
//===========================================================================
void cODEGenericBody::disableDynamics()
{
    if (m_ode_body == NULL) { return; }
    dBodyDisable(m_ode_body);
    dGeomDisable(m_ode_geom);
}


//===========================================================================
/*!
    This method renders the object graphically using OpenGL.

    \param    a_options  Rendering options.
*/
//===========================================================================
void cODEGenericBody::render(cRenderOptions& a_options)
{
    if (m_imageModel != NULL)
    {
        m_imageModel->renderSceneGraph(a_options);
    }


    /////////////////////////////////////////////////////////////////////////
    // Render parts that are always opaque
    /////////////////////////////////////////////////////////////////////////
    if (SECTION_RENDER_OPAQUE_PARTS_ONLY(a_options))
    {
        if (m_showDynamicCollisionModel)
        {
            glDisable(GL_LIGHTING);
            glLineWidth(1.0);
            glColor4fv(m_colorDynamicCollisionModel.getData());

            cTransform m_frameGL;
            m_frameGL.set(m_posOffsetDynColModel, m_rotOffsetDynColModel);
            glPushMatrix();
            glMultMatrixd( (const double *)m_frameGL.getData() );

            if (m_typeDynamicCollisionModel == ODE_MODEL_BOX)
            {
                double hx = m_paramDynColModel0 / 2.0;
                double hy = m_paramDynColModel1 / 2.0;
                double hz = m_paramDynColModel2 / 2.0;
                cDrawWireBox(-hx, hx, -hy, hy, -hz, hz);
            }
            else if (m_typeDynamicCollisionModel == ODE_MODEL_SPHERE)
            {
                // create sphere
                GLUquadricObj *sphere;
                sphere = gluNewQuadric ();

                // set rendering style
                gluQuadricDrawStyle (sphere, GLU_LINE);

                // set normal-rendering mode
                gluQuadricNormals (sphere, GLU_SMOOTH);

                // render a sphere
                gluSphere(sphere, m_paramDynColModel0, 16, 16);

                // delete our quadric object
                gluDeleteQuadric(sphere);
            }
            else if (m_typeDynamicCollisionModel == ODE_MODEL_CYLINDER)
            {
                // translate to center cylinder
                glTranslated(0.0, 0.0, -m_paramDynColModel1 / 2.0);

                // create cylinder
                GLUquadricObj *cylinder;
                cylinder = gluNewQuadric ();

                // set rendering style
                gluQuadricDrawStyle (cylinder, GLU_LINE);

                // set normal-rendering mode
                gluQuadricNormals (cylinder, GLU_SMOOTH);

                // render a sphere
                gluCylinder(cylinder, m_paramDynColModel0, m_paramDynColModel0, m_paramDynColModel1, 16, 16);

                // delete our quadric object
                gluDeleteQuadric(cylinder);
            }

            glEnable(GL_LIGHTING);
            glPopMatrix();
        }
    }
}


//===========================================================================
/*!
    This method updates globalPos and globalRot given the localPos and localRot
    of this object and its parents. \n

    If \a a_frameOnly is set to __false__, additional global positions such as
    vertex positions are computed (which can be time-consuming).

    \param  a_frameOnly  If __true___ then only frame is computed.
*/
//===========================================================================
void cODEGenericBody::updateGlobalPositions(const bool a_frameOnly)
{
    if (m_imageModel != NULL)
    {
        m_imageModel->computeGlobalPositions(a_frameOnly,
                                            m_globalPos,
                                            m_globalRot);
    }
};
