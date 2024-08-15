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
//------------------------------------------------------------------------------
#include <GLFW/glfw3.h>
//------------------------------------------------------------------------------
using namespace chai3d;
using namespace std;
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// DECLARED VARIABLES
//------------------------------------------------------------------------------

// a world that contains all objects of the virtual environment
cWorld* world;

// a camera to render the world in the window display
cCamera* camera;

// a viewport to display the scene viewed by the first camera
cViewport* viewport0 = nullptr;

// a camera attached to the endocope object
cCamera* cameraScope;

// a viewport to display the scene viewed by the second camera
cViewport* viewport1 = nullptr;

// a light source to illuminate the objects in the world
cDirectionalLight *light;

// a virtual object
cMultiMesh* heart;

// a virtual object
cMultiMesh* scope;

// a haptic device handler
cHapticDeviceHandler* handler;

// a pointer to the current haptic device
cGenericHapticDevicePtr hapticDevice;

// a virtual tool representing the haptic device in the scene
cToolCursor* tool;

// a colored background
cBackground* background;

// a font for rendering text
cFontPtr font;

// a label to display the rate [Hz] at which the simulation is running
cLabel* labelRates;

// a flag that indicates if the haptic simulation is currently running
bool simulationRunning = false;

// a flag that indicates if the haptic simulation has terminated
bool simulationFinished = true;

// a frequency counter to measure the simulation graphic rate
cFrequencyCounter freqCounterGraphics;

// a frequency counter to measure the simulation haptic rate
cFrequencyCounter freqCounterHaptics;

// haptic thread
cThread* hapticsThread;

// a first window
GLFWwindow* window0 = nullptr;
int framebuffer0W = 0;
int framebuffer0H = 0;
int window0W = 0;
int window0H = 0;

// a second window
GLFWwindow* window1 = nullptr;
int framebuffer1W = 0;
int framebuffer1H = 0;
int window1W = 0;
int window1H = 0;

// swap interval for the display context (vertical synchronization)
int swapInterval = 1;

// root resource path
string resourceRoot;


//------------------------------------------------------------------------------
// DECLARED MACROS
//------------------------------------------------------------------------------

// convert to resource path
#define RESOURCE_PATH(p)    (const char*)((resourceRoot+string(p)).c_str())


//------------------------------------------------------------------------------
// DECLARED FUNCTIONS
//------------------------------------------------------------------------------

// callback when an error GLFW occurs
void onErrorCallback(int a_error, const char* a_description);

// callback when a key is pressed
void onKeyCallback(GLFWwindow* a_window, int a_key, int a_scancode, int a_action, int a_mods);

// callback when the window 0 is resized
void onWindowSizeCallback0(GLFWwindow* a_window, int a_width, int a_height);

// callback when the window 1 is resized
void onWindowSizeCallback1(GLFWwindow* a_window, int a_width, int a_height);

// callback when the window 0 framebuffer is resized
void onFrameBufferSizeCallback0(GLFWwindow* a_window, int a_width, int a_height);

// callback when the window 1 framebuffer is resized
void onFrameBufferSizeCallback1(GLFWwindow* a_window, int a_width, int a_height);

// callback when window 0 content scaling is modified
void onWindowContentScaleCallback0(GLFWwindow* a_window, float a_xscale, float a_yscale);

// callback when window 1 content scaling is modified
void onWindowContentScaleCallback1(GLFWwindow* a_window, float a_xscale, float a_yscale);

// this function renders the scene
void renderGraphics0(void);

// this function renders the scene
void renderGraphics1(void);

// this function contains the main haptics simulation loop
void renderHaptics(void);

// this function closes the application
void close(void);


//==============================================================================
/*
    DEMO:   18-endoscope.cpp

    This example demonstrates the use of dual display contexts. In the first
    window we illustrate a tool exploring a virtual heart. A second camera 
    is attached at the extremity of the tools and the image rendered in a
    second viewport.
*/
//==============================================================================

int main(int argc, char* argv[])
{
    //--------------------------------------------------------------------------
    // INITIALIZATION
    //--------------------------------------------------------------------------

    cout << endl;
    cout << "-----------------------------------" << endl;
    cout << "CHAI3D" << endl;
    cout << "Demo: 18-endoscope" << endl;
    cout << "Copyright 2003-2024" << endl;
    cout << "-----------------------------------" << endl << endl << endl;
    cout << "Keyboard Options:" << endl << endl;
    cout << "[q] - Exit application" << endl;
    cout << endl << endl;

    // parse first arg to try and locate resources
    resourceRoot = string(argv[0]).substr(0,string(argv[0]).find_last_of("/\\")+1);


    //--------------------------------------------------------------------------
    // OPEN GL - WINDOW DISPLAY
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

    // compute desired size of window
    const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    int space = 10;
    int w = 0.5 * mode->height;
    int h = 0.5 * mode->height;
    int x0 = 0.5 * mode->width - w - space;
    int y0 = 0.5 * (mode->height - h);
    int x1 = 0.5 * mode->width + space;
    int y1 = 0.5 * (mode->height - h);

    // set OpenGL version
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

    // enable double buffering
    glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);

    // specify that window should be resized based on monitor content scale
    glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);

    ////////////////////////////////////////////////////////////////////////////
    // SETUP WINDOW 0
    ////////////////////////////////////////////////////////////////////////////

    // create display context
    window0 = glfwCreateWindow(w, h, "CHAI3D", NULL, NULL);
    if (!window0)
    {
        cout << "failed to create window" << endl;
        cSleepMs(1000);
        glfwTerminate();
        return 1;
    }

    // set GLFW key callback
    glfwSetKeyCallback(window0, onKeyCallback);

    // set GLFW window size callback
    glfwSetWindowSizeCallback(window0, onWindowSizeCallback0);

    // set GLFW framebuffer size callback
    glfwSetFramebufferSizeCallback(window0, onFrameBufferSizeCallback0);

    // set GLFW window content scaling callback
    glfwSetWindowContentScaleCallback(window0, onWindowContentScaleCallback0);

    // get width and height of window
    glfwGetFramebufferSize(window0, &framebuffer0W, &framebuffer0H);

    // set position of window
    glfwSetWindowPos(window0, x0, y0);

    // set window size
    glfwSetWindowSize(window0, w, h);

    // set GLFW current display context
    glfwMakeContextCurrent(window0);

    // set the desired number of samples to use for multisampling
    glfwWindowHint(GLFW_SAMPLES, 4);

    // set GLFW swap interval for the current display context
    glfwSwapInterval(swapInterval);


    ////////////////////////////////////////////////////////////////////////////
    // SETUP WINDOW 1
    ////////////////////////////////////////////////////////////////////////////

    // create display context and share GPU data with window 0
    window1 = glfwCreateWindow(w, h, "CHAI3D", NULL, window0);
    if (!window1)
    {
        cout << "failed to create window" << endl;
        cSleepMs(1000);
        glfwTerminate();
        return 1;
    }

    // set GLFW key callback
    glfwSetKeyCallback(window1, onKeyCallback);

    // set GLFW window size callback
    glfwSetWindowSizeCallback(window1, onWindowSizeCallback1);

    // set GLFW framebuffer size callback
    glfwSetFramebufferSizeCallback(window1, onFrameBufferSizeCallback1);

    // set GLFW window content scaling callback
    glfwSetWindowContentScaleCallback(window1, onWindowContentScaleCallback1);

    // get width and height of window
    glfwGetFramebufferSize(window1, &framebuffer1W, &framebuffer1H);

    // set position of window
    glfwSetWindowPos(window1, x1, y1);

    // set window size
    glfwSetWindowSize(window1, w, h);

    // set GLFW current display context
    glfwMakeContextCurrent(window1);

    // set the desired number of samples to use for multisampling
    glfwWindowHint(GLFW_SAMPLES, 4);

    // set GLFW swap interval for the current display context
    glfwSwapInterval(swapInterval);


    ////////////////////////////////////////////////////////////////////////////
    // GLEW
    ////////////////////////////////////////////////////////////////////////////

    // initialize GLEW library
#ifdef GLEW_VERSION
    if (glewInit() != GLEW_OK)
    {
        cout << "failed to initialize GLEW library" << endl;
        glfwTerminate();
        return 1;
    }
#endif


    //--------------------------------------------------------------------------
    // WORLD - CAMERA - LIGHTING
    //--------------------------------------------------------------------------

    // create a new world.
    world = new cWorld();

    // set the background color of the environment
    world->m_backgroundColor.setWhite();

    // create a camera and insert it into the virtual world
    camera = new cCamera(world);
    world->addChild(camera);

    // position and orient the camera
    camera->set(cVector3d(1.5, 0.0, 1.0),    // camera position (eye)
                cVector3d(0.0, 0.0, 0.0),    // lookat position (target)
                cVector3d(0.0, 0.0, 1.0));   // direction of the (up) vector

    // set the near and far clipping planes of the camera
    // anything in front or behind these clipping planes will not be rendered
    camera->setClippingPlanes(0.01, 100);

    // create a light source
    light = new cDirectionalLight(world);

    // add light to world
    world->addChild(light);

    // enable light source
    light->setEnabled(true);

    // define the direction of the light beam
    light->setDir(-1.0,-1.0, -1.0);

    // set lighting conditions
    light->m_ambient.set(0.4f, 0.4f, 0.4f);
    light->m_diffuse.set(0.8f, 0.8f, 0.8f);
    light->m_specular.set(1.0f, 1.0f, 1.0f);


    //--------------------------------------------------------------------------
    // HAPTIC DEVICES / TOOLS
    //--------------------------------------------------------------------------

    // create a haptic device handler
    handler = new cHapticDeviceHandler();

    // get access to the first available haptic device found
    handler->getDevice(hapticDevice, 0);

    // retrieve information about the current haptic device
    cHapticDeviceInfo hapticDeviceInfo = hapticDevice->getSpecifications();

    // create a tool (cursor) and insert into the world
    tool = new cToolCursor(world);
    world->addChild(tool);

    // connect the haptic device to the virtual tool
    tool->setHapticDevice(hapticDevice);

    // define the radius of the tool (sphere)
    double toolRadius = 0.01;

    // define a radius for the tool
    tool->setRadius(toolRadius);

    // hide the device sphere. only show proxy.
    tool->setShowContactPoints(false, false);

    // create a white cursor
    tool->m_hapticPoint->m_sphereProxy->m_material->setWhite();

    // map the physical workspace of the haptic device to a larger virtual workspace.
    tool->setWorkspaceRadius(1.0);

    // haptic forces are enabled only if small forces are first sent to the device;
    // this mode avoids the force spike that occurs when the application starts when 
    // the tool is located inside an object for instance. 
    tool->setWaitForSmallForce(true);




    //--------------------------------------------------------------------------
    // CREATE OBJECTS
    //--------------------------------------------------------------------------

    // read the scale factor between the physical workspace of the haptic
    // device and the virtual workspace defined for the tool
    double workspaceScaleFactor = tool->getWorkspaceScaleFactor();

    // properties
    double maxStiffness = hapticDeviceInfo.m_maxLinearStiffness / workspaceScaleFactor;


    /////////////////////////////////////////////////////////////////////////
    // OBJECT "HEART"
    /////////////////////////////////////////////////////////////////////////

    // create a virtual mesh
    heart = new cMultiMesh();

    // add object to world
    world->addChild(heart);

    // load an object file
    bool fileload;
    fileload = heart->loadFromFile(RESOURCE_PATH("../resources/models/heart/heart.3ds"));

    if (!fileload)
    {
        #if defined(_MSVC)
        fileload = heart->loadFromFile("../../../bin/resources/models/heart/heart.3ds");
        #endif
    }
    if (!fileload)
    {
        cout << "Error - 3D Model failed to load correctly." << endl;
        close();
        return (-1);
    }    

    // disable culling so that faces are rendered on both sides
    heart->setUseCulling(true);

    // scale model
    heart->scale(0.06);

    // compute collision detection algorithm
    heart->createAABBCollisionDetector(toolRadius);

    // define a default stiffness for the object
    heart->setStiffness(0.1 * maxStiffness, true);

    // use display list for faster rendering
    heart->setUseDisplayList(true);

    // position and orient object in scene
    heart->setLocalPos(0.0, -0.2, 0.2);
    heart->rotateExtrinsicEulerAnglesDeg(90, 0, 0, C_EULER_ORDER_XYZ);

    cMaterial mat;
    mat.setHapticTriangleSides(true, true);
    heart->setMaterial(mat);


    /////////////////////////////////////////////////////////////////////////
    // OBJECT "SCOPE"
    /////////////////////////////////////////////////////////////////////////

    // create a virtual mesh
    scope = new cMultiMesh();

    // attach scope to tool
    tool->m_image = scope;

    // load an object file
    fileload = scope->loadFromFile(RESOURCE_PATH("../resources/models/endoscope/endoscope.3ds"));

    if (!fileload)
    {
#if defined(_MSVC)
        fileload = scope->loadFromFile("../../../bin/resources/models/endoscope/endoscope.3ds");
#endif
    }
    if (!fileload)
    {
        cout << "Error - 3D Model failed to load correctly." << endl;
        close();
        return (-1);
    }    

    // disable culling so that faces are rendered on both sides
    scope->setUseCulling(false);

    // scale model
    scope->scale(0.1);

    // use display list for faster rendering
    scope->setUseDisplayList(true);

    // position object in scene
    scope->rotateExtrinsicEulerAnglesDeg(0, 0, 0, C_EULER_ORDER_XYZ);


    /////////////////////////////////////////////////////////////////////////
    // CAMERA "SCOPE"
    /////////////////////////////////////////////////////////////////////////

    // create a camera and insert it into the virtual world
    cameraScope = new cCamera(world);
    scope->addChild(cameraScope);

    // position and orient the camera
    cameraScope->setLocalPos(-0.03, 0.0, 0.1);

    // set the near and far clipping planes of the camera
    // anything in front or behind these clipping planes will not be rendered
    cameraScope->setClippingPlanes(0.01, 100);


    //--------------------------------------------------------------------------
    // WIDGETS
    //--------------------------------------------------------------------------

    // create a font
    font = NEW_CFONT_CALIBRI_20();
    
    // create a label to display the haptic and graphic rate of the simulation
    labelRates = new cLabel(font);
    labelRates->m_fontColor.setBlack();
    camera->m_frontLayer->addChild(labelRates);

    // create a background
    background = new cBackground();
    camera->m_backLayer->addChild(background);

    // set background properties
    background->setCornerColors(cColorf(1.0f, 1.0f, 1.0f),
                                cColorf(1.0f, 1.0f, 1.0f),
                                cColorf(0.9f, 0.9f, 0.9f),
                                cColorf(0.9f, 0.9f, 0.9f));

    // create a frontground for the endoscope
    cBackground* frontground = new cBackground();
    cameraScope->m_frontLayer->addChild(frontground);

    // load an texture map
    fileload = frontground->loadFromFile(RESOURCE_PATH("../resources/images/scope.png"));
    if (!fileload)
    {
#if defined(_MSVC)
        fileload = frontground->loadFromFile("../../../bin/resources/images/scope.png");
#endif
    }
    if (!fileload)
    {
        cout << "Error - Image failed to load correctly." << endl;
        close();
        return (-1);
    }


    //--------------------------------------------------------------------------
    // VIEWPORT DISPLAY
    //--------------------------------------------------------------------------

    // get content scale factor
    float contentScaleW0, contentScaleH0;
    glfwGetWindowContentScale(window0, &contentScaleW0, &contentScaleH0);

    // create a viewport to display the scene.
    viewport0 = new cViewport(camera, contentScaleW0, contentScaleH0);

    // get content scale factor
    float contentScaleW1, contentScaleH1;
    glfwGetWindowContentScale(window1, &contentScaleW1, &contentScaleH1);

    // create a viewport to display the scene.
    viewport1 = new cViewport(cameraScope, contentScaleW1, contentScaleH1);


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

    // main graphic loop
    while ((!glfwWindowShouldClose(window0)) && (!glfwWindowShouldClose(window1)))
    {
         // render graphics window 0
        renderGraphics0();

        // render graphics window 1
        renderGraphics1();

        // process events
        glfwPollEvents();
    }

    // close windows
    glfwDestroyWindow(window0);
    glfwDestroyWindow(window1);

    // terminate GLFW library
    glfwTerminate();

    // exit
    return 0;
}

//------------------------------------------------------------------------------

void onWindowSizeCallback0(GLFWwindow* a_window, int a_width, int a_height)
{
    // update window size
    window0W = a_width;
    window0H = a_height;

    // render scene
    renderGraphics0();
}

//------------------------------------------------------------------------------

void onWindowSizeCallback1(GLFWwindow* a_window, int a_width, int a_height)
{
    // update window size
    window1W = a_width;
    window1H = a_height;

    // render scene
    renderGraphics1();
}

//------------------------------------------------------------------------------

void onFrameBufferSizeCallback0(GLFWwindow* a_window, int a_width, int a_height)
{
    // update frame buffer size
    framebuffer0W = a_width;
    framebuffer0H = a_height;
}

//------------------------------------------------------------------------------

void onFrameBufferSizeCallback1(GLFWwindow* a_window, int a_width, int a_height)
{
    // update frame buffer size
    framebuffer1W = a_width;
    framebuffer1H = a_height;
}

//------------------------------------------------------------------------------

void onWindowContentScaleCallback0(GLFWwindow* a_window, float a_xscale, float a_yscale)
{
    // update window content scale factor
    viewport0->setContentScale(a_xscale, a_yscale);
}

//------------------------------------------------------------------------------

void onWindowContentScaleCallback1(GLFWwindow* a_window, float a_xscale, float a_yscale)
{
    // update window content scale factor
    viewport1->setContentScale(a_xscale, a_yscale);
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

void renderGraphics0(void)
{
    // sanity check
    if (viewport0 == nullptr) { return; }

    /////////////////////////////////////////////////////////////////////
    // UPDATE WIDGETS
    /////////////////////////////////////////////////////////////////////

    // get width and height of CHAI3D internal rendering buffer
    int displayW = viewport0->getDisplayWidth();
    int displayH = viewport0->getDisplayHeight();

    // update haptic and graphic rate data
    labelRates->setText(cStr(freqCounterGraphics.getFrequency(), 0) + " Hz / " +
        cStr(freqCounterHaptics.getFrequency(), 0) + " Hz");

    // update position of label
    labelRates->setLocalPos((int)(0.5 * (displayW - labelRates->getWidth())), 15);


    /////////////////////////////////////////////////////////////////////
    // RENDER SCENE
    /////////////////////////////////////////////////////////////////////

    // activate display context
    glfwMakeContextCurrent(window0);

    // update shadow maps (if any)
    world->updateShadowMaps(false, false);

    // render world
    viewport0->renderView(framebuffer0W, framebuffer0H);

    // wait until all GL commands are completed
    glFinish();

    // check for any OpenGL errors
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) cout << "Error: " << gluErrorString(error) << endl;

    // swap buffers
    glfwSwapBuffers(window0);

    // signal frequency counter
    freqCounterGraphics.signal(1);
}

//------------------------------------------------------------------------------

void renderGraphics1(void)
{
    // sanity check
    if (viewport1 == nullptr) { return; }

    /////////////////////////////////////////////////////////////////////
    // RENDER SCENE
    /////////////////////////////////////////////////////////////////////

    // activate display context
    glfwMakeContextCurrent(window1);

    // update shadow maps (if any)
    world->updateShadowMaps(false, false);

    // render world
    viewport1->renderView(framebuffer1W, framebuffer1H);

    // wait until all GL commands are completed
    glFinish();

    // check for any OpenGL errors
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) cout << "Error: " << gluErrorString(error) << endl;

    // swap buffers
    glfwSwapBuffers(window1);
}

//------------------------------------------------------------------------------

void renderHaptics(void)
{
    // simulation in now running
    simulationRunning  = true;
    simulationFinished = false;

    // start the haptic tool
    tool->start();

    // main haptic simulation loop
    while(simulationRunning)
    {
        /////////////////////////////////////////////////////////////////////////
        // HAPTIC RENDERING
        /////////////////////////////////////////////////////////////////////////

        // signal frequency counter
        freqCounterHaptics.signal(1);

        // compute global reference frames for each object
        world->computeGlobalPositions(true);

        // update position and orientation of tool
        tool->updateFromDevice();

        // compute interaction forces
        tool->computeInteractionForces();

        // send forces to haptic device
        tool->applyToDevice();
    }
    
    // exit haptics thread
    simulationFinished = true;
}

//------------------------------------------------------------------------------
