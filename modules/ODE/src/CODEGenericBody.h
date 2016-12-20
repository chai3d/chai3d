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
    \version   3.2.0 $Rev: 1917 $
*/
//===========================================================================

//---------------------------------------------------------------------------
#ifndef CODEGenericBodyH
#define CODEGenericBodyH
//---------------------------------------------------------------------------
#ifndef dDOUBLE
#define dDOUBLE
#endif
#include "ode/ode.h"
#include "chai3d.h"
//---------------------------------------------------------------------------

//===========================================================================
/*!
    \file       CODEGenericBody.h

    \brief
    <b> ODE Module </b> \n 
    ODE Generic Object.
*/
//===========================================================================

//---------------------------------------------------------------------------
class cODEWorld;
//---------------------------------------------------------------------------
//! ODE geometry used for dynamic collision computation.
enum cODEDynamicModelType
{
    ODE_MODEL_BOX,
    ODE_MODEL_SPHERE,
    ODE_MODEL_CYLINDER,
    ODE_MODEL_PLANE,
    ODE_MODEL_TRIMESH
};
//---------------------------------------------------------------------------

//===========================================================================
/*!
    \class      cODEGenericBody
    \ingroup    ODE

    \brief
    This class implements an ODE dynamic object.

    \details
    cODEGenericBody is a base class for modeling any ODE dynamic body.
*/
//===========================================================================
class cODEGenericBody : public chai3d::cGenericObject, public dBody
{
    //-----------------------------------------------------------------------
    // CONSTRUCTOR & DESTRUCTOR:
    //-----------------------------------------------------------------------

public:

    //! Constructor of cODEGenericBody.
    cODEGenericBody(cODEWorld* a_world) { initialize(a_world); }

    //! Destructor of cODEGenericBody.
    virtual ~cODEGenericBody() {};


    //-----------------------------------------------------------------------
    // PUBLIC METHODS - TRANSLATION AND ORIENTATION:
    //-----------------------------------------------------------------------

public:

    //! This method sets the local position of object.
    virtual void setLocalPos(const chai3d::cVector3d &a_position);

#ifdef C_USE_EIGEN
    //! This method sets the local position of this object.
    void setLocalPos(const Eigen::Vector3d& a_localPos)
    {
        setLocalPos(chai3d::cVector3d(a_localPos[0], a_localPos[1], a_localPos[2]));
    }
#endif

    //! This method sets the local position of this object.
    void setLocalPos(const double a_x = 0.0, 
        const double a_y = 0.0, 
        const double a_z = 0.0)
    {
        setLocalPos(chai3d::cVector3d(a_x, a_y, a_z));
    }

#ifdef C_USE_EIGEN
    //! This method sets the local rotation matrix for this object.
    inline void setLocalRot(const Eigen::Matrix3d a_localRot)
    {
        chai3d::cMatrix3d localRot;
        localRot.copyfrom(a_localRot);
        setLocalRot(localRot);
    }
#endif

    //! This method sets the orientation of this object.
    virtual void setLocalRot(const chai3d::cMatrix3d &a_rotation);

    //! This method updates the position and orientation from the ODE model to CHAI3D model.
    void updateBodyPosition(void);


    //-----------------------------------------------------------------------
    // PUBLIC METHODS - EXTERNAL FORCES:
    //-----------------------------------------------------------------------

public:

    //! This method applies an external force at the center of mass of this object.
    void addExternalForce(const chai3d::cVector3d& a_force);

    //! This method applies an external torque at the center of mass of of this object.
    void addExternalTorque(const chai3d::cVector3d& a_torque);

    //! This method applies an external force at a given position described in global coordinates to this object.
    void addExternalForceAtPoint(const chai3d::cVector3d& a_force,
                                 const chai3d::cVector3d& a_pos);

    //-----------------------------------------------------------------------
    // PUBLIC METHODS - DYNAMIC PROPERTIES:
    //-----------------------------------------------------------------------

public:

    //! This method assigns a value for the mass of this object.
    void setMass(const double a_mass);

    //! This method returns the mass value for this object.
    double getMass() const { return (m_mass); }

    //! This method enables dynamics for this object.
    void enableDynamics();

    //! This method disables dynamics for this object.
    void disableDynamics();

    //! This method returns __true__ if the object is dynamically static, __false__ otherwise.
    bool isStatic() const { return (m_static); } 

    //! This method creates a dynamic model of the object for modeling collisions between objects.
    virtual void buildDynamicModel() {};

    //! This method creates a dynamic box representation for modeling collisions between objects.
    void createDynamicBoundingBox(const bool a_staticObject = false);

    //! This method creates a dynamic sphere representation for modeling collisions between objects.
    void createDynamicSphere(const double a_radius, 
        const bool a_staticObject = false,
        const chai3d::cVector3d& a_offsetPos = chai3d::cVector3d(0.0, 0.0, 0.0),
        const chai3d::cMatrix3d& a_offsetRot = chai3d::cIdentity3d());

    //! This method creates a dynamic box representation for modeling collisions between objects.
    void createDynamicBox(const double a_lengthX, 
        const double a_lengthY, 
        const double a_lengthZ,
        const  bool a_staticObject = false,
        const chai3d::cVector3d& a_offsetPos = chai3d::cVector3d(0.0, 0.0, 0.0),
        const chai3d::cMatrix3d& a_offsetRot = chai3d::cIdentity3d());

    //! This method creates a dynamic capsule representation for modeling collisions between objects.
    void createDynamicCapsule(const double a_radius, 
        const double a_length,
        const bool a_staticObject = false,
        const chai3d::cVector3d& a_offsetPos = chai3d::cVector3d(0.0, 0.0, 0.0),
        const chai3d::cMatrix3d& a_offsetRot = chai3d::cIdentity3d());

    //! This method creates a dynamic caped cylinder representation for modeling collisions between objects.
    void createDynamicCylinder(const double a_radius,
        const double a_length,
        const bool a_staticObject = false,
        const chai3d::cVector3d& a_offsetPos = chai3d::cVector3d(0.0, 0.0, 0.0),
        const chai3d::cMatrix3d& a_offsetRot = chai3d::cIdentity3d());

    //! This method creates a static plane representation for modeling collisions between objects.
    void createStaticPlane(const chai3d::cVector3d& a_position,
        const chai3d::cVector3d& a_normal);

    //! This method creates a triangle mesh representation for modeling collisions between objects.
    void createDynamicMesh(const bool a_staticObject = false,
        const chai3d::cVector3d& a_offsetPos = chai3d::cVector3d(0.0, 0.0, 0.0),
        const chai3d::cMatrix3d& a_offsetRot = chai3d::cIdentity3d());


    //-----------------------------------------------------------------------
    // PUBLIC METHODS - COLLISION DETETCTION:
    //-----------------------------------------------------------------------

public:

    //! This method computes all collisions between a segment passed as argument and the body image of this object.
    virtual bool computeCollisionDetection(const chai3d::cVector3d& a_segmentPointA,
                                           const chai3d::cVector3d& a_segmentPointB,
                                           chai3d::cCollisionRecorder& a_recorder,
                                           chai3d::cCollisionSettings& a_settings);


    //-----------------------------------------------------------------------
    // PUBLIC METHODS - BODY IMAGE AND DISPLAY PROPERTIES:
    //-----------------------------------------------------------------------

public:

    //! This method sets a CHAI3D body image for this object.
    void setImageModel(chai3d::cGenericObject* a_imageModel);

    //! This method returns a pointer to the CHAI3D body image.
    chai3d::cGenericObject* getImageModel() { return (m_imageModel); }

    //! This method displays or hides the collision model.
    void setShowDynamicCollisionModel(const bool a_show) { m_showDynamicCollisionModel = a_show; }

    //! This method returns the display status of the collision model.
    bool getShowDynamicCollisionModel() { return (m_showDynamicCollisionModel); }


    //-----------------------------------------------------------------------
    // PUBLIC MEMBERS:
    //-----------------------------------------------------------------------

public:

    //! Parent world.
    cODEWorld* m_ODEWorld;

    //! Object used to represent the geometry and graphical properties of the object.
    chai3d::cGenericObject* m_imageModel;

    //! ODE body.
    dBodyID m_ode_body;

    //! ODE body geometry.
    dGeomID m_ode_geom;

    //! Color used to render collision model.
    chai3d::cColorf m_colorDynamicCollisionModel;


    //-----------------------------------------------------------------------
    // PROTECTED METHODS:
    //-----------------------------------------------------------------------

protected:

    //! This method updates the global position frame of this object.
    virtual void updateGlobalPositions(const bool a_frameOnly);

    //! This method renders the object graphically using OpenGL.
    virtual void render(chai3d::cRenderOptions& a_options);

    //! This method builds table of triangles and vertices for ODE multiMesh representation.
    int buildMeshTable(chai3d::cMultiMesh* a_multiMesh);

    //! This method builds table of triangles and vertices for ODE mesh representation.
    int buildMeshTable(chai3d::cMesh* a_mesh);

    //! This method initializes the ODE object.
    void initialize(cODEWorld* a_world);


    //-----------------------------------------------------------------------
    // PROTECTED MEMBERS:
    //-----------------------------------------------------------------------

protected:

    //! if __true__ then ODE object is static and does not move. 
    bool m_static;

    //! ODE body mass matrix.
    dMass m_ode_mass;

    //! Mass of object units: [kg].
    double m_mass;

    //! ODE vertices (for triangle mesh models) - Do not use doubles since not supported under ODE! 
    float* m_vertices;

    //! ODE indices (for triangle mesh models).
    int* m_vertexIndices;

    //! ODE previous tri mesh position and orientation.
    double m_prevTransform[16];

    //! ODE triangle mesh ID.
    dTriMeshDataID m_ode_triMeshDataID;

    //! Enable/Disable graphical representation of collision model.
    bool m_showDynamicCollisionModel;

    //! Dynamic model type.
    cODEDynamicModelType m_typeDynamicCollisionModel;

    /*! 
        Variables which store the parameters of the collision model.
        Depending of the dynamic model type, these value correspond to
        lengths or radius.
    */
    double m_paramDynColModel0;
    double m_paramDynColModel1;
    double m_paramDynColModel2;
    chai3d::cVector3d m_posOffsetDynColModel;
    chai3d::cMatrix3d m_rotOffsetDynColModel;
};


//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
