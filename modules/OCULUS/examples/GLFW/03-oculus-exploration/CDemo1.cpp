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
    \version   3.2.0 $Rev: 1637 $
*/
//===========================================================================

//---------------------------------------------------------------------------
#include "CDemo1.h"
using namespace std;
//---------------------------------------------------------------------------

//===========================================================================
/*!
    Constructor of cDemo1.
*/
//===========================================================================
cDemo1::cDemo1(const string a_resourceRoot,
               const int a_numDevices,
               shared_ptr<cGenericHapticDevice> a_hapticDevice0,
               shared_ptr<cGenericHapticDevice> a_hapticDevice1):cGenericDemo(a_resourceRoot, a_numDevices, a_hapticDevice0, a_hapticDevice1)
{
    cMaterial matBase;
    matBase.setGrayLevel(0.3);
    matBase.setStiffness(1500);

    m_ODEBase0 = new cODEGenericBody(m_ODEWorld);
    cMesh* base0 = new cMesh();
    cCreateBox(base0, 0.005, 0.15, 0.01);
    m_ODEBase0->createDynamicBox(0.005, 0.15, 0.01, true);
    base0->createAABBCollisionDetector(m_toolRadius);
    base0->setMaterial(matBase);
    m_ODEBase0->setImageModel(base0);
    m_ODEBase0->setLocalPos(0.013, 0.0, 0.0);

    m_ODEBase1 = new cODEGenericBody(m_ODEWorld);
    cMesh* base1 = new cMesh();
    cCreateBox(base1, 0.005, 0.15, 0.01);
    m_ODEBase1->createDynamicBox(0.005, 0.15, 0.01, true);
    base1->createAABBCollisionDetector(m_toolRadius);
    base1->setMaterial(matBase);
    m_ODEBase1->setImageModel(base1);
    m_ODEBase1->setLocalPos(-0.013, 0.0, 0.0);


    m_ODEBase2 = new cODEGenericBody(m_ODEWorld);
    cMesh* base2 = new cMesh();
    cCreateBox(base2, 0.02, 0.005, 0.01);
    m_ODEBase2->createDynamicBox(0.02, 0.005, 0.01, true);
    base2->createAABBCollisionDetector(m_toolRadius);
    base2->setMaterial(matBase);
    m_ODEBase2->setImageModel(base2);
    m_ODEBase2->setLocalPos(0.0,-0.0725, 0.0);

    m_ODEBase3 = new cODEGenericBody(m_ODEWorld);
    cMesh* base3 = new cMesh();
    cCreateBox(base3, 0.02, 0.005, 0.01);
    m_ODEBase3->createDynamicBox(0.02, 0.005, 0.01, true);
    base3->createAABBCollisionDetector(m_toolRadius);
    base3->setMaterial(matBase);
    m_ODEBase3->setImageModel(base3);
    m_ODEBase3->setLocalPos(0.0,-0.0455, 0.0);

    m_ODEBase4 = new cODEGenericBody(m_ODEWorld);
    cMesh* base4 = new cMesh();
    cCreateBox(base4, 0.02, 0.005, 0.01);
    m_ODEBase4->createDynamicBox(0.02, 0.005, 0.01, true);
    base4->createAABBCollisionDetector(m_toolRadius);
    base4->setMaterial(matBase);
    m_ODEBase4->setImageModel(base4);
    m_ODEBase4->setLocalPos(0.0, 0.0725, 0.0);

    m_ODEBase5 = new cODEGenericBody(m_ODEWorld);
    cMesh* base5 = new cMesh();
    cCreateBox(base5, 0.02, 0.005, 0.01);
    m_ODEBase5->createDynamicBox(0.02, 0.005, 0.01, true);
    base5->createAABBCollisionDetector(m_toolRadius);
    base5->setMaterial(matBase);
    m_ODEBase5->setImageModel(base5);
    m_ODEBase5->setLocalPos(0.0, 0.0455, 0.0);

    // create a new ODE object that is automatically added to the ODE world
    m_ODEBody0 = new cODEGenericBody(m_ODEWorld);
    m_ODEBody1 = new cODEGenericBody(m_ODEWorld);
    m_ODEBody2 = new cODEGenericBody(m_ODEWorld);

    // create a virtual mesh  that will be used for the geometry
    // representation of the dynamic body
    cMesh* object0 = new cMesh();
    cMesh* object1 = new cMesh();
    cMesh* object2 = new cMesh();

    // crate a cube mesh
    double boxSize = 0.02;
    cCreateBox(object0, boxSize, boxSize, boxSize);
    cCreateBox(object1, boxSize, boxSize, boxSize);
    cCreateBox(object2, boxSize, boxSize, boxSize);

    object0->createAABBCollisionDetector(m_toolRadius);
    object1->createAABBCollisionDetector(m_toolRadius);
    object2->createAABBCollisionDetector(m_toolRadius);

    // define some material properties for each cube
    cMaterial mat;
    mat.setGrayLightSlate();
    mat.m_specular.set(0.0, 0.0, 0.0);
    mat.setStiffness(1000);
    mat.setDynamicFriction(1.0);
    mat.setStaticFriction(0.9);
    object0->setMaterial(mat);
    object1->setMaterial(mat);
    object2->setMaterial(mat);

    // add mesh to ODE object
    m_ODEBody0->setImageModel(object0);
    m_ODEBody1->setImageModel(object1);
    m_ODEBody2->setImageModel(object2);

    // create a dynamic model of the ODE object. Here we decide to use a box just like
    // the object mesh we just defined
    m_ODEBody0->createDynamicBox(boxSize, boxSize, boxSize);
    m_ODEBody1->createDynamicBox(boxSize, boxSize, boxSize);
    m_ODEBody2->createDynamicBox(boxSize, boxSize, boxSize);

    // define some mass properties for each cube
    m_ODEBody0->setMass(0.02);
    m_ODEBody1->setMass(0.02);
    m_ODEBody2->setMass(0.02);

    // initialize
    init();
};


//===========================================================================
/*!
    Set stiffness of environment

    \param  a_stiffness  Stiffness value [N/m]
*/
//===========================================================================
void cDemo1::setStiffness(double a_stiffness)
{
    // set ground
    m_ground->setStiffness(a_stiffness, true);
    
    // set objects
    m_ODEBody0->m_imageModel->setStiffness(a_stiffness);
    m_ODEBody1->m_imageModel->setStiffness(a_stiffness);
    m_ODEBody2->m_imageModel->setStiffness(a_stiffness);

    m_ODEBase0->m_imageModel->setStiffness(a_stiffness);
    m_ODEBase1->m_imageModel->setStiffness(a_stiffness);
    m_ODEBase2->m_imageModel->setStiffness(a_stiffness);
    m_ODEBase3->m_imageModel->setStiffness(a_stiffness);
    m_ODEBase4->m_imageModel->setStiffness(a_stiffness);
    m_ODEBase5->m_imageModel->setStiffness(a_stiffness);
};


//===========================================================================
/*!
    Initialize position of objects
*/
//===========================================================================
void cDemo1::init()
{
    // set position of each cube
    cVector3d tmpvct;
    m_ODEBody0->setLocalPos(0.04,-0.05, 0.04);
    m_ODEBody1->setLocalPos(0.04, 0.00, 0.04);
    m_ODEBody2->setLocalPos(0.04, 0.05, 0.04);
}
