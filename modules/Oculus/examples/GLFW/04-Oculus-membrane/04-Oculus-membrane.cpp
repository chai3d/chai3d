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
    \version   3.3.0
*/
//==============================================================================

//------------------------------------------------------------------------------
#include "chai3d.h"
#include "GEL3D.h"
//------------------------------------------------------------------------------
#include <GLFW/glfw3.h>
//------------------------------------------------------------------------------
#include "COculus.h"
//------------------------------------------------------------------------------
using namespace chai3d;
using namespace std;
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// CHAI3D
//------------------------------------------------------------------------------

// a world that contains all objects of the virtual environment
cWorld* world;

// a camera to render the world in the window display
cCamera* camera;

// a light source
cDirectionalLight *light;

// a haptic device handler
cHapticDeviceHandler* handler;

// a pointer to the current haptic device
cGenericHapticDevicePtr hapticDevice;

// force scale factor
double deviceForceScale;

// scale factor between the device workspace and cursor workspace
double workspaceScaleFactor;

// desired workspace radius of the virtual cursor
double cursorWorkspaceRadius;

// indicates if the haptic simulation currently running
bool simulationRunning = false;

// indicates if the haptic simulation has terminated
bool simulationFinished = true;

// frequency counter to measure the simulation haptic rate
cFrequencyCounter frequencyCounter;

// haptic thread
cThread *hapticsThread;


//---------------------------------------------------------------------------
// GEL
//---------------------------------------------------------------------------

// deformable world
cGELWorld* defWorld;

// object mesh
cGELMesh* defObject;

// dynamic nodes
cGELSkeletonNode* nodes[10][10];

// haptic device model
cShapeSphere* device;
double deviceRadius;

// radius of the dynamic model sphere (GEM)
double radius;

// stiffness properties between the haptic device tool and the model (GEM)
double stiffness;

// scale factor
double scale = 0.5;


//------------------------------------------------------------------------------
// OCULUS RIFT
//------------------------------------------------------------------------------

// oculus device
cOculusDevice* oculusDisplay;


//------------------------------------------------------------------------------
// DECLARED FUNCTIONS
//------------------------------------------------------------------------------

// callback when an error GLFW occurs
void onErrorCallback(int a_error, const char* a_description);

// callback when a key is pressed
void onKeyCallback(GLFWwindow* a_window, int a_key, int a_scancode, int a_action, int a_mods);

// function that closes the application
void close(void);

// main haptics simulation loop
void renderHaptics(void);

// compute forces between tool and environment
cVector3d computeForce(const cVector3d& a_cursor,
                       double a_cursorRadius,
                       const cVector3d& a_spherePos,
                       double a_radius,
                       double a_stiffness);


//==============================================================================
/*
    DEMO:   04-membrane.cpp

    This example illustrates how to use GEL to create a environment which
    contains deformable objects.
*/
//==============================================================================

int main(int argc, char **argv)
{
    //--------------------------------------------------------------------------
    // INITIALIZATION
    //--------------------------------------------------------------------------

    cout << endl;
    cout << "-----------------------------------" << endl;
    cout << "CHAI3D" << endl;
    cout << "Demo: 04-membrane" << endl;
    cout << "Copyright 2003-2024" << endl;
    cout << "-----------------------------------" << endl << endl << endl;
    cout << "Keyboard Options:" << endl << endl;
    cout << "[ ] - Recenter view point" << endl;
    cout << "[f] - Enable/Disable full screen mode" << endl;
    cout << "[q] - Exit application" << endl;
    cout << endl << endl;

    // get current path
    bool fileload;
    string currentpath = cGetCurrentPath();


    //--------------------------------------------------------------------------
    // SETUP DISPLAY CONTEXT
    //--------------------------------------------------------------------------

    // initialize GLFW library
    if (!glfwInit())
    {
        cout << "failed initialization" << endl;
        cSleepMs(1000);
        return 1;
    }

    // set GLFW error callback
    glfwSetErrorCallback(onErrorCallback);

    // set OpenGL version
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

    // enable double buffering
    glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);

    // set the desired number of samples to use for multisampling
    glfwWindowHint(GLFW_SAMPLES, 4);

    // specify that window should be resized based on monitor content scale
    glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);

    // create display context
    GLFWwindow* window = glfwCreateWindow(640, 480, "CHAI3D", NULL, NULL);
    if (!window)
    {
        cout << "failed to create window" << endl;
        cSleepMs(1000);
        glfwTerminate();
        return 1;
    }

    // set key callback
    glfwSetKeyCallback(window, onKeyCallback);

    // set current display context
    glfwMakeContextCurrent(window);

    // sets the swap interval for the current display context
    glfwSwapInterval(0);

#ifdef GLEW_VERSION
    // initialize GLEW library
    if (glewInit() != GLEW_OK)
    {
        glfwTerminate();
        return 1;
    }
#endif


    //--------------------------------------------------------------------------
    // WORLD - CAMERA - LIGHTING
    //--------------------------------------------------------------------------

    // create a new world.hh
    world = new cWorld();

    // create a camera and insert it into the virtual world
    camera = new cCamera(world);
    world->addChild(camera);

    // position and orient the camera
    camera->set(cVector3d(0.6, 0.0, 0.4),    // camera position (eye)
                cVector3d(0.0, 0.0, 0.4),    // lookat position (target)
                cVector3d(0.0, 0.0, 1.0));   // direction of the (up) vector

    // set the near and far clipping planes of the camera
    camera->setClippingPlanes(0.01, 20.0);

    // enable multi-pass rendering to handle transparent objects
    camera->setUseMultipassTransparency(true);

    // create a directional light source
    light = new cDirectionalLight(world);

    // insert light source inside world
    world->addChild(light);

    // enable light source
    light->setEnabled(true);
    // define direction of light beam
    light->setDir(0.0, 0.0,-1.0); 


    //--------------------------------------------------------------------------
    // HAPTIC DEVICES / TOOLS
    //--------------------------------------------------------------------------

    // create a haptic device handler
    handler = new cHapticDeviceHandler();

    // get access to the first available haptic device found
    handler->getDevice(hapticDevice, 0);

    // retrieve information about the current haptic device
    cHapticDeviceInfo hapticDeviceInfo = hapticDevice->getSpecifications();

    // open connection to haptic device
    hapticDevice->open();

    // desired workspace radius of the cursor
    cursorWorkspaceRadius = 0.7;

    // read the scale factor between the physical workspace of the haptic
    // device and the virtual workspace defined for the tool
    workspaceScaleFactor = cursorWorkspaceRadius / hapticDeviceInfo.m_workspaceRadius;

    // define a scale factor between the force perceived at the cursor and the
    // forces actually sent to the haptic device
    deviceForceScale = 5.0;

    // create a large sphere that represents the haptic device
    deviceRadius = scale * 0.1;
    device = new cShapeSphere(deviceRadius);
    world->addChild(device);
    device->m_material->setWhite();
    device->m_material->setShininess(100);

    // interaction stiffness between tool and deformable model 
    stiffness = 100;


    //-----------------------------------------------------------------------
    // COMPOSE THE VIRTUAL SCENE
    //-----------------------------------------------------------------------

    // create a world which supports deformable object
    defWorld = new cGELWorld();
    world->addChild(defWorld);

    // create a deformable mesh
    defObject = new cGELMesh();
    defWorld->m_gelMeshes.push_front(defObject);

    // load model
    fileload = defObject->loadFromFile(currentpath + "../resources/models/box/box.obj");
    if (!fileload)
    {
        cout << "Error - 3D Model failed to load correctly." << endl;
        close();
        return (-1);
    }

    // scale object
    defObject->scale(scale);

    // set some material color on the object
    cMaterial mat;
    mat.setWhite();
    mat.setShininess(100);
    defObject->setMaterial(mat, true);

    // let's create a some environment mapping
    shared_ptr<cTexture2d> texture(new cTexture2d());
    fileload = texture->loadFromFile(currentpath + "../resources/images/shadow.jpg");
    if (!fileload)
    {
         cout << "Error - Texture failed to load correctly." << endl;
        close();
        return (-1);
    }

    // enable environmental texturing
    texture->setEnvironmentMode(GL_DECAL);
    texture->setSphericalMappingEnabled(true);

    // assign and enable texturing
    defObject->setTexture(texture, true);
    defObject->setUseTexture(true, true);

    // set object to be transparent
    defObject->setTransparencyLevel(0.65, true, true);
    
    // build dynamic vertices
    defObject->buildVertices();

    // set default properties for skeleton nodes
    cGELSkeletonNode::s_default_radius        = scale * 0.05;  // [m]
    cGELSkeletonNode::s_default_kDampingPos   = 2.5;
    cGELSkeletonNode::s_default_kDampingRot   = 0.6;
    cGELSkeletonNode::s_default_mass          = 0.002; // [kg]
    cGELSkeletonNode::s_default_showFrame     = true;
    cGELSkeletonNode::s_default_color.setBlueCornflower();
    cGELSkeletonNode::s_default_useGravity    = true;
    cGELSkeletonNode::s_default_gravity.set(0.00, 0.00,-9.81);
    radius = cGELSkeletonNode::s_default_radius;

    // use internal skeleton as deformable model
    defObject->m_useSkeletonModel = true;

    // create an array of nodes
    for (int y=0; y<10; y++)
    {
        for (int x=0; x<10; x++)
        {
            cGELSkeletonNode* newNode = new cGELSkeletonNode();
            nodes[x][y] = newNode;
            defObject->m_nodes.push_front(newNode);
            newNode->m_pos.set( (-scale * 0.45 + scale * 0.1*(double)x), (-scale * 0.43 + scale * 0.1*(double)y), 0.0);
        }
    }

    // set corner nodes as fixed
    nodes[0][0]->m_fixed = true;
    nodes[0][9]->m_fixed = true;
    nodes[9][0]->m_fixed = true;
    nodes[9][9]->m_fixed = true;

    // set default physical properties for links
    cGELSkeletonLink::s_default_kSpringElongation = 25.0;  // [N/m]
    cGELSkeletonLink::s_default_kSpringFlexion    = 0.5;   // [Nm/RAD]
    cGELSkeletonLink::s_default_kSpringTorsion    = 0.1;   // [Nm/RAD]
    cGELSkeletonLink::s_default_color.setBlueCornflower();

    // create links between nodes
    for (int y=0; y<9; y++)
    {
        for (int x=0; x<9; x++)
        {
            cGELSkeletonLink* newLinkX0 = new cGELSkeletonLink(nodes[x+0][y+0], nodes[x+1][y+0]);
            cGELSkeletonLink* newLinkX1 = new cGELSkeletonLink(nodes[x+0][y+1], nodes[x+1][y+1]);
            cGELSkeletonLink* newLinkY0 = new cGELSkeletonLink(nodes[x+0][y+0], nodes[x+0][y+1]);
            cGELSkeletonLink* newLinkY1 = new cGELSkeletonLink(nodes[x+1][y+0], nodes[x+1][y+1]);
            defObject->m_links.push_front(newLinkX0);
            defObject->m_links.push_front(newLinkX1);
            defObject->m_links.push_front(newLinkY0);
            defObject->m_links.push_front(newLinkY1);
        }
    }

    // connect skin (mesh) to skeleton (GEM)
    defObject->connectVerticesToSkeleton(false);

    // show/hide underlying dynamic skeleton model
    defObject->m_showSkeletonModel = false;


    //--------------------------------------------------------------------------
    // SETUP OCULUS DISPLAY
    //--------------------------------------------------------------------------

    // setup oculus display
    oculusDisplay = new cOculusDevice();

    // initialize oculus
    if (!oculusDisplay->initialize(camera, true, 0.5))
    {
        cout << "Error - Oculus display failed to initialize." << endl;
        close();
        return (-1);
    }

    // set window size
    glfwSetWindowSize(window, oculusDisplay->getWindowWidth(), oculusDisplay->getWindowHeight());

    // recenter oculus
    oculusDisplay->recenterPose();


    //--------------------------------------------------------------------------
    // START HAPTIC SIMULATION THREAD
    //--------------------------------------------------------------------------

    // create a thread which starts the main haptics rendering loop
    hapticsThread = new cThread();
    hapticsThread->start(renderHaptics, CTHREAD_PRIORITY_HAPTICS);

    // setup callback when application exits
    atexit(close);


    //--------------------------------------------------------------------------
    // MAIN GRAPHIC LOOP
    //--------------------------------------------------------------------------

    // main graphic rendering loop
    while (!glfwWindowShouldClose(window))
    {
        // update skins of deformable objects
        defWorld->updateSkins(true);

        // render scene to oculus display
        oculusDisplay->render();

        // finalize
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // cleanup
    glfwDestroyWindow(window);

    // exit glfw
    glfwTerminate();

    return (0);
}

//------------------------------------------------------------------------------

void onErrorCallback(int a_error, const char* a_description)
{
    cout << "Error: " << a_description << endl;
}

//------------------------------------------------------------------------------

void onKeyCallback(GLFWwindow* a_window, int a_key, int a_scancode, int a_action, int a_mods)
{
    // filter calls that only include a key press
    if ((a_action != GLFW_PRESS) && (a_action != GLFW_REPEAT))
    {
        return;
    }

    // option - exit
    else if ((a_key == GLFW_KEY_ESCAPE) || (a_key == GLFW_KEY_Q))
    {
        glfwSetWindowShouldClose(a_window, GLFW_TRUE);
    }

    // option - spacebar
    else if (a_key == GLFW_KEY_SPACE)
    {
        oculusDisplay->recenterPose();
    }
}

//------------------------------------------------------------------------------

void close(void)
{
    // stop the simulation
    simulationRunning = false;

    // wait for graphics and haptics loops to terminate
    while (!simulationFinished) { cSleepMs(100); }

    // close haptic device
    hapticDevice->close();

    // delete resources
    delete hapticsThread;
    delete world;
    delete handler;
}

//------------------------------------------------------------------------------

void renderHaptics(void)
{
    // initialize frequency counter
    frequencyCounter.reset();

    // initialize precision clock
    cPrecisionClock clock;
    clock.reset();

    // simulation in now running
    simulationRunning  = true;
    simulationFinished = false;

    // main haptic simulation loop
    while(simulationRunning)
    {
        // stop clock
        double time = cMin(0.001, clock.stop());

        // restart clock
        clock.start(true);

        // read position from haptic device
        cVector3d pos;
        hapticDevice->getPosition(pos);
        pos.mul(workspaceScaleFactor);
        device->setLocalPos(pos);

        // clear all external forces
        defWorld->clearExternalForces();

        // compute reaction forces
        cVector3d force(0.0, 0.0, 0.0);
        for (int y=0; y<10; y++)
        {
            for (int x=0; x<10; x++)
            {
               cVector3d nodePos = nodes[x][y]->m_pos;
               cVector3d f = computeForce(pos, deviceRadius, nodePos, radius, stiffness);
               cVector3d tmpfrc = -1.0 * f;
               nodes[x][y]->setExternalForce(tmpfrc);
               force.add(f);
            }
        }

        // integrate dynamics
        defWorld->updateDynamics(0.5 * time);

        // scale force
        force.mul(deviceForceScale / workspaceScaleFactor);

        // send forces to haptic device
        hapticDevice->setForce(force);

        // update frequency counter
        frequencyCounter.signal(1);
    }

    // exit haptics thread
    simulationFinished = true;
}

//------------------------------------------------------------------------------

cVector3d computeForce(const cVector3d& a_cursor,
                       double a_cursorRadius,
                       const cVector3d& a_spherePos,
                       double a_radius,
                       double a_stiffness)
{
    // compute the reaction forces between the tool and the ith sphere.
    cVector3d force;
    force.zero();
    cVector3d vSphereCursor = a_cursor - a_spherePos;

    // check if both objects are intersecting
    if (vSphereCursor.length() < 0.0000001)
    {
        return (force);
    }

    if (vSphereCursor.length() > (a_cursorRadius + a_radius))
    {
        return (force);
    }

    // compute penetration distance between tool and surface of sphere
    double penetrationDistance = (a_cursorRadius + a_radius) - vSphereCursor.length();
    cVector3d forceDirection = cNormalize(vSphereCursor);
    force = cMul( penetrationDistance * a_stiffness, forceDirection);

    // return result
    return (force);
}

//---------------------------------------------------------------------------
