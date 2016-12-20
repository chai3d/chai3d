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
    \author    Francois Conti
    \version   3.2.0 $Rev: 1659 $
*/
//==============================================================================

//------------------------------------------------------------------------------
#include "chai3d.h"
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

// a virtual tool representing the haptic device in the scene
cToolCursor* tool;

// a mesh model
cMultiMesh* object;

// model of earth
cMesh* earth;

// model of starts
cMesh* globe;

// indicates if the haptic simulation currently running
bool simulationRunning = false;

// indicates if the haptic simulation has terminated
bool simulationFinished = false;

// frequency counter to measure the simulation haptic rate
cFrequencyCounter frequencyCounter;

// haptic thread
cThread *hapticsThread;


//------------------------------------------------------------------------------
// OCULUS RIFT
//------------------------------------------------------------------------------

// display context
cOVRRenderContext renderContext;

// oculus device
cOVRDevice oculusVR;


//------------------------------------------------------------------------------
// DECLARED MACROS
//------------------------------------------------------------------------------

// convert to resource path
#define RESOURCE_PATH(p)    (char*)((resourceRoot+string(p)).c_str())


//------------------------------------------------------------------------------
// DECLARED FUNCTIONS
//------------------------------------------------------------------------------

// callback when an error GLFW occurs
void errorCallback(int error, const char* a_description);

// callback when a key is pressed
void keyCallback(GLFWwindow* a_window, int a_key, int a_scancode, int a_action, int a_mods);

// function that closes the application
void close(void);

// main haptics simulation loop
void updateHaptics(void);


//==============================================================================
/*
    DEMO:   02-space.cpp

    This example illustrates how to the oculus to create an immersive
    environment in space. A spherical map is used to model the stars.
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
    cout << "Demo: 02-space" << endl;
    cout << "Copyright 2003-2016" << endl;
    cout << "-----------------------------------" << endl << endl << endl;
    cout << "Keyboard Options:" << endl << endl;
    cout << "[ ] - Recenter view point" << endl;
    cout << "[q] - Exit application" << endl;
    cout << endl << endl;

    // parse first arg to try and locate resources
    string resourceRoot = string(argv[0]).substr(0, string(argv[0]).find_last_of("/\\") + 1);


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

    // set error callback
    glfwSetErrorCallback(errorCallback);

    // set OpenGL version
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    // create display context
    GLFWwindow* window = glfwCreateWindow(640, 480, "My Title", NULL, NULL);
    if (!window)
    {
        cout << "failed to create window" << endl;
        cSleepMs(1000);
        glfwTerminate();
        return 1;
    }

    // set key callback
    glfwSetKeyCallback(window, keyCallback);

    // set current display context
    glfwMakeContextCurrent(window);

    // sets the swap interval for the current display context
    glfwSwapInterval(0);

#ifdef GLEW_VERSION
    // initialize GLEW library
    if (glewInit() != GLEW_OK)
    {
        oculusVR.destroyVR();
        renderContext.destroy();
        glfwTerminate();
        return 1;
    }
#endif

    // initialize oculus
    if (!oculusVR.initVR())
    {
        cout << "failed to initialize Oculus" << endl;
        cSleepMs(1000);
        glfwTerminate();
        return 1;
    }

    // get oculus display resolution
    ovrSizei hmdResolution = oculusVR.getResolution();

    // setup mirror display on computer screen
    ovrSizei windowSize = { hmdResolution.w / 2, hmdResolution.h / 2 };

    // inialize buffers
    if (!oculusVR.initVRBuffers(windowSize.w, windowSize.h))
    {
        cout << "failed to initialize Oculus buffers" << endl;
        cSleepMs(1000);
        oculusVR.destroyVR();
        renderContext.destroy();
        glfwTerminate();
        return 1;
    }

    // set window size
    glfwSetWindowSize(window, windowSize.w, windowSize.h);


    //--------------------------------------------------------------------------
    // WORLD - CAMERA - LIGHTING
    //--------------------------------------------------------------------------

    // create a new world.
    world = new cWorld();

    // set the background color of the environment
    // the color is defined by its (R,G,B) components.
    world->m_backgroundColor.setWhite();

    // create a camera and insert it into the virtual world
    camera = new cCamera(world);
    world->addChild(camera);

    // position and orient the camera
    camera->set(cVector3d(0.1, 0.0, 0.0),    // camera position (eye)
        cVector3d(0.0, 0.0, 0.0),    // lookat position (target)
        cVector3d(0.0, 0.0, 1.0));   // direction of the "up" vector

                                     // set the near and far clipping planes of the camera
                                     // anything in front/behind these clipping planes will not be rendered
    camera->setClippingPlanes(0.1, 100.0);

    // create a light source
    light = new cDirectionalLight(world);

    // attach light to camera
    camera->addChild(light);

    // enable light source
    light->setEnabled(true);

    // define the direction of the light beam
    light->setDir(-3.0, -0.5, 0.0);

    // set lighting conditions
    light->m_ambient.set(0.4f, 0.4f, 0.4f);
    light->m_diffuse.set(0.8f, 0.8f, 0.8f);
    light->m_specular.set(1.0f, 1.0f, 1.0f);


    //--------------------------------------------------------------------------
    // HAPTIC DEVICES / TOOLS
    //--------------------------------------------------------------------------

    // create a haptic device handler
    handler = new cHapticDeviceHandler();

    // get access to the first available haptic device
    handler->getDevice(hapticDevice, 0);

    // retrieve information about the current haptic device
    cHapticDeviceInfo hapticDeviceInfo = hapticDevice->getSpecifications();

    // create a 3D tool and add it to the world
    tool = new cToolCursor(world);
    camera->addChild(tool);

    // set position
    tool->setLocalPos(-1.0, 0.0, 0.0);

    // connect the haptic device to the tool
    tool->setHapticDevice(hapticDevice);

    // define the radius of the tool (sphere)
    double toolRadius = 0.02;

    // define a radius for the tool
    tool->setRadius(toolRadius);

    // enable if objects in the scene are going to rotate of translate
    // or possibly collide against the tool. If the environment
    // is entirely static, you can set this parameter to "false"
    tool->enableDynamicObjects(true);

    // map the physical workspace of the haptic device to a larger virtual workspace.
    tool->setWorkspaceRadius(1.0);

    // haptic forces are enabled only if small forces are first sent to the device;
    // this mode avoids the force spike that occurs when the application starts when 
    // the tool is located inside an object for instance. 
    tool->setWaitForSmallForce(true);

    // start the haptic tool
    tool->start();


    //--------------------------------------------------------------------------
    // CREATE OBJECT
    //--------------------------------------------------------------------------

    // read the scale factor between the physical workspace of the haptic
    // device and the virtual workspace defined for the tool
    double workspaceScaleFactor = tool->getWorkspaceScaleFactor();

    // stiffness properties
    double maxStiffness = hapticDeviceInfo.m_maxLinearStiffness / workspaceScaleFactor;

    // create a virtual mesh
    object = new cMultiMesh();

    // add object to world
    world->addChild(object);

    // set the position of the object at the center of the world
    object->setLocalPos(-1.0, 0.0, 0.0);

    // rotate the object 90 degrees
    object->rotateAboutGlobalAxisDeg(cVector3d(0, 0, 1), 90);

    // load an object file
    bool fileload;
    fileload = object->loadFromFile(RESOURCE_PATH("../resources/models/hubble/hubble.3ds"));
    if (!fileload)
    {
#if defined(_MSVC)
        fileload = object->loadFromFile("../../../bin/resources/models/hubble/hubble.3ds");
#endif
    }
    if (!fileload)
    {
        cout << "Error - 3D Model failed to load correctly." << endl;
        close();
        return (-1);
    }

    // compute a boundary box
    object->computeBoundaryBox(true);

    // get dimensions of object
    double size = cSub(object->getBoundaryMax(), object->getBoundaryMin()).length();

    // resize object to screen
    if (size > 0)
    {
        object->scale(3.0 * tool->getWorkspaceRadius() / size);
    }

    // compute collision detection algorithm
    object->createAABBCollisionDetector(toolRadius);

    // enable haptic rendering on both sides of triangles
    cMaterial mat;
    mat.setHapticTriangleSides(true, true);
    object->setMaterial(mat);

    // define some environmental texture mapping
    cTexture2dPtr texture = cTexture2d::create();

    // load texture file
    fileload = texture->loadFromFile(RESOURCE_PATH("../resources/images/chrome.jpg"));
    if (!fileload)
    {
#if defined(_MSVC)
        fileload = texture->loadFromFile("../../../bin/resources/images/chrome.jpg");
#endif
    }
    if (!fileload)
    {
        cout << "Error - Texture image failed to load correctly." << endl;
        close();
        return (-1);
    }

    // enable spherical mapping
    texture->setSphericalMappingEnabled(true);

    // assign texture to object
    object->setTexture(texture, true);

    // enable texture mapping
    object->setUseTexture(true, true);

    // disable culling
    object->setUseCulling(false, true);

    // enable display list for faster graphic rendering
    object->setUseDisplayList(true);

    // define a default stiffness for the object
    object->setStiffness(maxStiffness, true);

    // define some haptic friction properties
    object->setFriction(0.1, 0.2, true);


    //--------------------------------------------------------------------------
    // CREATE ENVIRONMENT MAP
    //--------------------------------------------------------------------------

    // create a virtual mesh
    globe = new cMesh();

    // add object to world
    world->addChild(globe);

    // set the position of the object at the center of the world
    globe->setLocalPos(0.0, 0.0, 0.0);

    // create cube
    cCreateSphere(globe, 75.0, 36, 36);
    globe->setUseDisplayList(true);
    globe->deleteCollisionDetector();

    // create a texture
    cTexture2dPtr textureSpace = cTexture2d::create();

    fileload = textureSpace->loadFromFile(RESOURCE_PATH("../resources/images/space.jpg"));
    if (!fileload)
    {
#if defined(_MSVC)
        fileload = textureSpace->loadFromFile("../../../bin/resources/images/space.jpg");
#endif
    }
    if (!fileload)
    {
        cout << "Error - Texture image failed to load correctly." << endl;
        close();
        return (-1);
    }

    // apply texture to object
    globe->setTexture(textureSpace);

    // enable texture rendering 
    globe->setUseTexture(true);

    // Since we don't need to see our polygons from both sides, we enable culling.
    globe->setUseCulling(false);

    // disable material properties and lighting
    globe->setUseMaterial(false);


    //--------------------------------------------------------------------------
    // START SIMULATION
    //--------------------------------------------------------------------------

    // create a thread which starts the main haptics rendering loop
    hapticsThread = new cThread();
    hapticsThread->start(updateHaptics, CTHREAD_PRIORITY_HAPTICS);

    // setup callback when application exits
    atexit(close);


    //--------------------------------------------------------------------------
    // MAIN GRAPHIC LOOP
    //--------------------------------------------------------------------------

    // recenter oculus
    oculusVR.recenterPose();

    // main graphic rendering loop
    while (!glfwWindowShouldClose(window) && !simulationFinished)
    {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        // start rendering
        oculusVR.onRenderStart();

        // render frame for each eye
        for (int eyeIndex = 0; eyeIndex < ovrEye_Count; eyeIndex++)
        {
            // retrieve projection and modelview matrix from oculus
            cTransform projectionMatrix, modelViewMatrix;
            oculusVR.onEyeRender(eyeIndex, projectionMatrix, modelViewMatrix);

            camera->m_useCustomProjectionMatrix = true;
            camera->m_projectionMatrix = projectionMatrix;

            camera->m_useCustomModelViewMatrix = true;
            camera->m_modelViewMatrix = modelViewMatrix;

            // render world
            ovrSizei size = oculusVR.getEyeTextureSize(eyeIndex);
            camera->renderView(size.w, size.h, C_STEREO_LEFT_EYE, false);

            // finalize rendering  
            oculusVR.onEyeRenderFinish(eyeIndex);
        }

        // update frames
        oculusVR.submitFrame();
        oculusVR.blitMirror();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // cleanup
    oculusVR.destroyVR();
    renderContext.destroy();

    glfwDestroyWindow(window);

    // exit glfw
    glfwTerminate();

    return (0);
}

//------------------------------------------------------------------------------

void errorCallback(int a_error, const char* a_description)
{
    cout << "Error: " << a_description << endl;
}

//------------------------------------------------------------------------------

void keyCallback(GLFWwindow* a_window, int a_key, int a_scancode, int a_action, int a_mods)
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
        oculusVR.recenterPose();
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
    tool->stop();

    // delete resources
    delete hapticsThread;
    delete world;
    delete handler;
}

//------------------------------------------------------------------------------

void updateHaptics(void)
{
    // angular velocity
    cVector3d angVel(0, 0, 0.1);
    cVector3d linVel(0, 0.1, 0);

    // reset clock
    cPrecisionClock clock;
    clock.reset();

    // simulation in now running
    simulationRunning = true;
    simulationFinished = false;

    // reset tool
    tool->initialize();


    // main haptic simulation loop
    while (simulationRunning)
    {
        /////////////////////////////////////////////////////////////////////
        // SIMULATION TIME
        /////////////////////////////////////////////////////////////////////

        // stop the simulation clock
        clock.stop();

        // read the time increment in seconds
        double timeInterval = clock.getCurrentTimeSeconds();

        // restart the simulation clock
        clock.reset();
        clock.start();

        // update frequency counter
        frequencyCounter.signal(1);


        /////////////////////////////////////////////////////////////////////
        // HAPTIC FORCE COMPUTATION
        /////////////////////////////////////////////////////////////////////

        // compute global reference frames for each object
        world->computeGlobalPositions(true);

        // update position and orientation of tool
        tool->updateFromDevice();

        // compute interaction forces
        tool->computeInteractionForces();

        // send forces to haptic device
        tool->applyToDevice();


        /////////////////////////////////////////////////////////////////////
        // DYNAMIC SIMULATION
        /////////////////////////////////////////////////////////////////////

        // get position of cursor in global coordinates
        cVector3d toolPos = tool->getDeviceGlobalPos();

        // get position of object in global coordinates
        cVector3d objectPos = object->getGlobalPos();

        // compute a vector from the center of mass of the object (point of rotation) to the tool
        cVector3d v = cSub(toolPos, objectPos);

        // compute angular acceleration based on the interaction forces
        // between the tool and the object
        cVector3d angAcc(0, 0, 0);
        cVector3d linAcc(0, 0, 0);
        cVector3d forceTool(0, 0, 0);

        // compute force that centers object at origin
        cVector3d force = -0.2 * (objectPos - cVector3d(-1.2, 0.0, 0.0));

        if (v.length() > 0.0)
        {
            // get the last force applied to the cursor in global coordinates
            // we negate the result to obtain the opposite force that is applied on the
            // object
            forceTool = -tool->getDeviceGlobalForce();

            // compute the effective force that contributes to rotating the object.
            cVector3d f = forceTool - cProject(forceTool, v);

            // compute the resulting torque
            cVector3d torque = cMul(v.length(), cCross(cNormalize(v), f));

            // update angular acceleration
            const double INERTIA = 0.4;
            angAcc = (1.0 / INERTIA) * torque;
        }

        // update linear acceleration
        const double MASS = 1.0;
        linAcc = (1.0 / MASS) * (force + forceTool);

        // update angular velocity
        angVel.add(timeInterval * angAcc);

        // update linear velocity
        linVel.add(timeInterval * linAcc);

        // set a threshold on the rotational velocity term
        const double MAX_ANG_VEL = 10.0;
        double vel = angVel.length();
        if (vel > MAX_ANG_VEL)
        {
            angVel.mul(MAX_ANG_VEL / vel);
        }

        // add some damping too
        const double DAMPING = 0.1;
        angVel.mul(1.0 - DAMPING * timeInterval);
        linVel.mul(1.0 - DAMPING * timeInterval);

        // if user switch is pressed, set velocity to zero
        if (tool->getUserSwitch(0) == 1)
        {
            angVel.zero();
            linVel.zero();
        }

        // compute the next rotation configuration of the object
        if (angVel.length() > C_SMALL)
        {
            object->rotateAboutGlobalAxisRad(cNormalize(angVel), timeInterval * angVel.length());
        }

        // compute the next position of the object
        cVector3d nextPos = object->getLocalPos() + (timeInterval * linVel);
        object->setLocalPos(nextPos);
    }

    // exit haptics thread
    simulationFinished = true;
}

//------------------------------------------------------------------------------
