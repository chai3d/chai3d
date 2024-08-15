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
#include "COculus.h"
#include "CODE.h"
//------------------------------------------------------------------------------
#include "CDemo1.h"
#include "CDemo2.h"
#include "CDemo3.h"
#include "CDemo4.h"
#include "CGenericDemo.h"
//------------------------------------------------------------------------------
using namespace chai3d;
using namespace std;
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// CHAI3D
//------------------------------------------------------------------------------

// a haptic device handler
cHapticDeviceHandler* handler;

// a pointer to the current haptic device
shared_ptr<cGenericHapticDevice> m_hapticDevice0;
shared_ptr<cGenericHapticDevice> m_hapticDevice1;

// flag to indicate if the haptic simulation currently running
bool simulationRunning = false;

// flag to indicate if the haptic simulation has terminated
bool simulationFinished = true;

// haptic thread
cThread *hapticsThread;


//---------------------------------------------------------------------------
// DEMOS
//---------------------------------------------------------------------------

//! currently active camera
cGenericDemo* m_demo;

//! Demos
cDemo1* m_demo1;
cDemo2* m_demo2;
cDemo3* m_demo3;
cDemo4* m_demo4;


//------------------------------------------------------------------------------
// OCULUS RIFT
//------------------------------------------------------------------------------

// oculus device
cOculusDevice* oculusDisplay;


//------------------------------------------------------------------------------
// DECLARED FUNCTIONS
//------------------------------------------------------------------------------

// function that closes the application
void close(void);

// main haptics simulation loop
void renderHaptics(void);

// callback when an error GLFW occurs
void onErrorCallback(int a_error, const char* a_description);

// callback when a key is pressed
void onKeyCallback(GLFWwindow* a_window, int a_key, int a_scancode, int a_action, int a_mods);

// initialize demos
void initDemo1();
void initDemo2();
void initDemo3();
void initDemo4();


//==============================================================================
/*
    DEMO:   03-exploration.cpp

    This example illustrates how to use ODE to create a environment which
    contains dynamic objects.
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
    cout << "Demo: 03-exploration" << endl;
    cout << "Copyright 2003-2024" << endl;
    cout << "-----------------------------------" << endl << endl << endl;
    cout << "Keyboard Options:" << endl << endl;
    cout << "[ ] - Recenter view point" << endl;
    cout << "[1] - Select Demo 1" << endl;
    cout << "[2] - Select Demo 2" << endl;
    cout << "[3] - Select Demo 3" << endl;
    cout << "[4] - Select Demo 4" << endl;
    cout << "[q] - Exit application" << endl;
    cout << endl << endl;


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


    //-----------------------------------------------------------------------
    // HAPTIC DEVICES 
    //-----------------------------------------------------------------------

    // create a haptic device handler
    handler = new cHapticDeviceHandler();

    // get number of haptic devices
    int numDevices = handler->getNumDevices();

    // default stiffness of scene objects
    double maxStiffness = 1000.0;

    // get access to the haptic devices found
    if (numDevices > 0)
    {
        handler->getDevice(m_hapticDevice0, 0);
        maxStiffness = cMin(maxStiffness, 0.5 * m_hapticDevice0->getSpecifications().m_maxLinearStiffness);
    }

    if (numDevices > 1)
    {
        handler->getDevice(m_hapticDevice1, 1);
        maxStiffness = cMin(maxStiffness, 0.5 * m_hapticDevice1->getSpecifications().m_maxLinearStiffness);
    }


    //-----------------------------------------------------------------------
    // DEMOS
    //-----------------------------------------------------------------------

    // get current path
    string currentpath = cGetCurrentPath();

    // setup demos
    m_demo1 = new cDemo1(currentpath, numDevices, m_hapticDevice0, m_hapticDevice1);
    m_demo2 = new cDemo2(currentpath, numDevices, m_hapticDevice0, m_hapticDevice1);
    m_demo3 = new cDemo3(currentpath, numDevices, m_hapticDevice0, m_hapticDevice1);
    m_demo4 = new cDemo4(currentpath, numDevices, m_hapticDevice0, m_hapticDevice1);

    // set object stiffness in demos
    m_demo1->setStiffness(maxStiffness);
    m_demo2->setStiffness(maxStiffness);
    m_demo3->setStiffness(maxStiffness);
    m_demo4->setStiffness(maxStiffness);

    // initialize demo 1
    initDemo1();


    //--------------------------------------------------------------------------
    // SETUP OCULUS DISPLAY
    //--------------------------------------------------------------------------

    // setup oculus display
    oculusDisplay = new cOculusDevice();

    // initialize oculus
    if (!oculusDisplay->initialize(NULL, true, 0.5))
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
        // set current camera
        oculusDisplay->setCamera(m_demo->m_camera);



        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        // update shadow maps (if any)
        m_demo->m_world->updateShadowMaps(false, false);

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

    // option - demo 1
    else if (a_key == GLFW_KEY_1)
    {
        initDemo1();
    }

    // option - demo 2
    else if (a_key == GLFW_KEY_2)
    {
        initDemo2();
    }

    // option - demo 3
    else if (a_key == GLFW_KEY_3)
    {
        initDemo3();
    }

    // option - demo 4
    else if (a_key == GLFW_KEY_4)
    {
        initDemo4();
    }
}

//---------------------------------------------------------------------------

void initDemo1()
{
    m_demo = m_demo1;
    m_demo->init();
}

//---------------------------------------------------------------------------

void initDemo2()
{
    m_demo = m_demo2;
    m_demo->init();
}

//---------------------------------------------------------------------------

void initDemo3()
{
    m_demo = m_demo3;
    m_demo->init();
}

//---------------------------------------------------------------------------

void initDemo4()
{
    m_demo = m_demo4;
    m_demo->init();
}

//---------------------------------------------------------------------------

void close(void)
{
    // stop the simulation
    simulationRunning = false;

    // wait for graphics and haptics loops to terminate
    while (!simulationFinished) { cSleepMs(100); }

    // delete resources
    delete hapticsThread;
    delete m_demo1;
    delete m_demo2;
    delete m_demo3;
    delete m_demo4;
    delete handler;
}

//------------------------------------------------------------------------------

void renderHaptics(void)
{
    // simulation in now running
    simulationRunning = true;

    // main haptic simulation loop
    while(simulationRunning)
    {
        m_demo->renderHaptics();
    }

    // shutdown haptic devices
    if (m_demo->m_tool0 != NULL)
    {
        m_demo->m_tool0->stop();
    }
    if (m_demo->m_tool1 != NULL)
    {
        m_demo->m_tool1->stop();
    }

    // exit haptics thread
    simulationFinished = true;
}

//------------------------------------------------------------------------------
