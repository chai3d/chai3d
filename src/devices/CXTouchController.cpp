////////////////////////////////////////////////////////////////////////////////
//
//   Software License Agreement (BSD License)
//   Copyright (c) 2003-2024, CHAI3D
//   (www.chai3d.org)
//
//   All rights reserved.
//
//   Redistribution and use in source and binary forms, with or without
//   modification, are permitted provided that the following conditions
//   are met:
//
//   * Redistributions of source code must retain the above copyright
//   notice, this list of conditions and the following disclaimer.
//
//   * Redistributions in binary form must reproduce the above
//   copyright notice, this list of conditions and the following
//   disclaimer in the documentation and/or other materials provided
//   with the distribution.
//
//   * Neither the name of CHAI3D nor the names of its contributors may
//   be used to endorse or promote products derived from this software
//   without specific prior written permission.
//
//   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
//   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
//   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
//   FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
//   COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
//   INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
//   BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
//   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
//   CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
//   LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
//   ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
//   POSSIBILITY OF SUCH DAMAGE. 
//
////////////////////////////////////////////////////////////////////////////////

//------------------------------------------------------------------------------
#include "devices/CXTouchController.h"
//------------------------------------------------------------------------------
#include "math/CMaths.h"
#include "RtMidi.h"
//------------------------------------------------------------------------------
using namespace std;
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
namespace chai3d {
//------------------------------------------------------------------------------


////////////////////////////////////////////////////////////////////////////////
///
///  MIDI callback.
///
///  \param a_deltatime  Time stamp.
///  \param a_message    Message data.
///  \param a_userData   User data.
///
////////////////////////////////////////////////////////////////////////////////
void XTouchInterfaceCallback(double a_deltatime, std::vector< unsigned char > *a_message, void *a_userData)
{
    // get pointer to object
    cXTouchController* touchInterface = (cXTouchController*)a_userData;

    // forward MIDI message to object
    touchInterface->callbackMidiIn(a_deltatime, a_message);
}


////////////////////////////////////////////////////////////////////////////////
///
///  Constructor of cXTouchController.
///
////////////////////////////////////////////////////////////////////////////////
cXTouchController::cXTouchController(unsigned int a_deviceNumber)
{
    // initialize members
    m_layer = 0;
    m_model = C_XTOUCH_NOT_DETECTED;

    // declare variables
    unsigned int numPorts;
    unsigned int index, counter;

    ////////////////////////////////////////////////////////////////////////////
    // SETUP MIDI IN
    ////////////////////////////////////////////////////////////////////////////

    // create midi object for incoming communication
    midiin = new RtMidiIn();

    // assign callback
    ((RtMidiIn*)(midiin))->setCallback(&XTouchInterfaceCallback, this);

    // get number of ports
    numPorts = ((RtMidiIn*)(midiin))->getPortCount();

    // initialize counter
    counter = 0;

    // scan ports
    bool foundMidiIn = false;
    index = 0;
    while ((!foundMidiIn) && (index < numPorts))
    {
        // read model
        std::string portName = ((RtMidiIn*)(midiin))->getPortName(index);
        if (portName.find("X-TOUCH COMPACT") != std::string::npos)
        {
            if (a_deviceNumber == counter)
            {
                m_portNumberIn = index;
                foundMidiIn = true;
            }
            counter++;
        }
        else if (portName.find("X-TOUCH MINI") != std::string::npos)
        {
            if (a_deviceNumber == counter)
            {
                m_portNumberIn = index;
                foundMidiIn = true;
            }
            counter++;
        }

        // increment counter
        index++;
    }

    ////////////////////////////////////////////////////////////////////////////
    // SETUP MIDI OUT
    ////////////////////////////////////////////////////////////////////////////

    // create midi object for outgoing communication
    midiout = new RtMidiOut();

    // get number of ports
    numPorts = ((RtMidiOut*)(midiout))->getPortCount();

    // initialize counter
    counter = 0;

    // scan ports
    bool foundMidiOut = false;
    index = 0;
    while ((!foundMidiOut) && (index < numPorts))
    {
        // read model
        std::string portName = ((RtMidiOut*)(midiout))->getPortName(index);
        if (portName.find("X-TOUCH COMPACT") != std::string::npos)
        {
            if (a_deviceNumber == counter)
            {
                m_model = C_XTOUCH_COMPACT;
                m_portNumberOut = index;
                foundMidiOut = true;
            }
            counter++;
        }
        else if (portName.find("X-TOUCH MINI") != std::string::npos)
        {
            if (a_deviceNumber == counter)
            {
                m_model = C_XTOUCH_MINI;
                m_portNumberOut = index;
                foundMidiOut = true;
            }
            counter++;
        }

        // increment counter
        index++;
    }

    ////////////////////////////////////////////////////////////////////////////
    // FINALIZE
    ////////////////////////////////////////////////////////////////////////////

    if (foundMidiIn && foundMidiOut)
    {
        m_deviceAvailable = true;
    }
};


////////////////////////////////////////////////////////////////////////////////
///
///  Destructor of cXTouchController.
///
////////////////////////////////////////////////////////////////////////////////
cXTouchController::~cXTouchController()
{
    // close device
    close();
};


////////////////////////////////////////////////////////////////////////////////
///
///  This method opens a connection to this device.
///
///  \return 
///  Number of x-touch devices availble
///
////////////////////////////////////////////////////////////////////////////////
unsigned int cXTouchController::getNumDevices()
{
    // create a connection
    RtMidiIn* midi = new RtMidiIn();

    // get number of ports
    unsigned int numPorts = midi->getPortCount();

    // initialize counter
    unsigned int counter = 0;

    // scan ports
    for (unsigned int i = 0; i < numPorts; i++)
    {
        // read model
        std::string portName = midi->getPortName(i);
        if (portName == "X-TOUCH COMPACT " + cStr(i))
        {
            counter++;
        }
        else if (portName == "X-TOUCH MINI " + cStr(i))
        {
            counter++;
        }
    }

    // close midi connection
    midi->closePort();

    // return result
    return counter;
}


////////////////////////////////////////////////////////////////////////////////
///
///  This method opens a connection to this device.
///
///  \return 
///  __true__ if operation succeeds, __false__ otherwise.
///
////////////////////////////////////////////////////////////////////////////////
bool cXTouchController::open()
{
    if ((m_deviceAvailable) && (!m_deviceReady))
    {
        ((RtMidiIn*)(midiin))->openPort(m_portNumberIn);
        ((RtMidiIn*)(midiout))->openPort(m_portNumberOut);

        m_deviceReady = true;

        // initialize interface
        initialize();

        return true;
    }
    else
    {
        return false;
    }
}


////////////////////////////////////////////////////////////////////////////////
///
///  This method closes a connection to this device.
///
///  \return 
///  __true__ if operation succeeds, __false__ otherwise.
///
////////////////////////////////////////////////////////////////////////////////
bool cXTouchController::close()
{
    if (m_deviceReady)
    {
        ((RtMidiIn*)(midiin))->closePort();
        ((RtMidiOut*)(midiout))->closePort();
        
        m_deviceReady = false;

        return true;
    }
    else
    {
        return false;
    }
}


////////////////////////////////////////////////////////////////////////////////
///
///  This method iniaitalizes the table to its default state.
///
////////////////////////////////////////////////////////////////////////////////
void cXTouchController::initialize()
{
    ////////////////////////////////////////////////////////////////////////////
    // INITITALIZE VARIABLES
    ////////////////////////////////////////////////////////////////////////////

    for (int i = 0; i < C_XTOUCH_MAX_SLIDERS; i++)
    {
        m_sliderValue[i] = 0.0;
        m_sliderRange[i][0] = 0.0;
        m_sliderRange[i][1] = 1.0;
    }

    for (int i = 0; i < C_XTOUCH_MAX_DIALS; i++)
    {
        m_dialValue[i] = 0.0;
        m_dialRange[i][0] = 0.0;
        m_dialRange[i][1] = 1.0;
    }

    for (int i = 0; i < C_XTOUCH_MAX_BUTTONS; i++)
    {
        m_buttonState[i] = false;
        m_buttonBehavior[i] = 0;
        m_buttonMode[i] = 0;
    }

    ////////////////////////////////////////////////////////////////////////////
    // INITITALIZE X-TOUCH
    ////////////////////////////////////////////////////////////////////////////

    // x-touch mini
    if (m_model == C_XTOUCH_MINI)
    {
        // set layer A
        setLayerA();

        // set standard mode
        sendMidi3(176, 127, 0);

        // initialize all dials
        setAllDialsValue(0.0);

        // initialize all lights
        setAllButtonsLightOff();

        // set momentary mode
        setAllButtonsMomentaryMode();
    }

    // x-touch compact
    else if (m_model == C_XTOUCH_COMPACT)
    {
        // set standard mode
        sendMidi3(177, 127, 0);

        // initialize all sliders
        setAllSlidersValue(0.0);

        // initialize all dials
        setAllDialsValue(0.0);

        // initialize all lights
        setAllButtonsLightOff();

        // set momentary mode
        setAllButtonsMomentaryMode();

        // set layer A
        setLayerA();
    }
}


////////////////////////////////////////////////////////////////////////////////
///
///  This method selects the desired layer (A or B)
///
///  \param a_layer  Layer number: 0 or 1.
///
////////////////////////////////////////////////////////////////////////////////
void cXTouchController::setLayer(int a_layer)
{
    // sanity check
    if (!m_deviceReady) { return; }

    // store layer
    m_layer = cClamp(a_layer, 0, 1);

    // x-touch mini
    if (m_model == C_XTOUCH_MINI)
    {
        sendMidi2(192, a_layer);
    }

    // x-touch compact
    else if (m_model == C_XTOUCH_COMPACT)
    {
        sendMidi2(193, a_layer);
    }
}


////////////////////////////////////////////////////////////////////////////////
///
///  This method returns the number of sliders.
///
///  \return 
///  Number of sliders available.
///
////////////////////////////////////////////////////////////////////////////////
int cXTouchController::getNumSliders()
{
    // x-touch mini
    if (m_model == C_XTOUCH_MINI)
    {
        return 2;
    }

    // x-touch compact
    else if (m_model == C_XTOUCH_COMPACT)
    {
        return 18;
    }

    // non supported device
    else
    {
        return 0;
    }
}


////////////////////////////////////////////////////////////////////////////////
///
///  This method returns the value of a slider.
///
///  \param a_index  Index number of slider.
///
///  \return 
///  Value of slider position.
///
////////////////////////////////////////////////////////////////////////////////
double cXTouchController::getSliderValue(int a_index)
{
    // sanity check
    if (!m_deviceReady || (!cContains(a_index, 0, getNumSliders() - 1))) { return 0.0; }

    // return slider value
    return (m_sliderValue[a_index]);
}


////////////////////////////////////////////////////////////////////////////////
///
///  This method sets the value of a slider.
///
///  \param a_index  Index number of slider.
///  \param a_value  Desired value
///
////////////////////////////////////////////////////////////////////////////////
void cXTouchController::setSliderValue(int a_index, double a_value)
{
    // sanity check
    if (!m_deviceReady || (!cContains(a_index, 0, getNumSliders() - 1))) { return; }

    // compute midi value
    m_sliderValue[a_index] = cClamp(a_value, m_sliderRange[a_index][0], m_sliderRange[a_index][1]);
    unsigned char value = convertTo(m_sliderValue[a_index], m_sliderRange[a_index][0], m_sliderRange[a_index][1]);

    // x-touch mini
    if (m_model == C_XTOUCH_MINI)
    {
    }

    // x-touch compact
    else if (m_model == C_XTOUCH_COMPACT)
    {
        if (cContains(a_index, 0, 8))
        {
            setLayerA();
            sendMidi3(176, a_index + 1, value);
        }

        else if (cContains(a_index, 9, 17))
        {
            setLayerB();
            sendMidi3(176, a_index + 19, value);
        }
    }
}


////////////////////////////////////////////////////////////////////////////////
///
///  This method sets all sliders to a given value.
///
///  \param a_value  Desired value
///
////////////////////////////////////////////////////////////////////////////////
void cXTouchController::setAllSlidersValue(double a_value)
{
    for (int i = 0; i < getNumSliders(); i++)
    {
        setSliderValue(i, a_value);
    }
}


////////////////////////////////////////////////////////////////////////////////
///
///  This method returns the number of dials.
///
///  \return 
///  Number of dials available.
///
////////////////////////////////////////////////////////////////////////////////
int cXTouchController::getNumDials()
{
    // x-touch mini
    if (m_model == C_XTOUCH_MINI)
    {
        return 16;
    }

    // x-touch compact
    else if (m_model == C_XTOUCH_COMPACT)
    {
        return 32;
    }

    // non supported device
    else
    {
        return 0;
    }
}


////////////////////////////////////////////////////////////////////////////////
///
///  This method returns the value of a dial.
///
///  \param a_index  Index value.
///
///  \return 
///  Current value of the dial.
///
////////////////////////////////////////////////////////////////////////////////
double cXTouchController::getDialValue(int a_index)
{
    // sanity check
    if (!m_deviceReady || (!cContains(a_index, 0, getNumDials() - 1))) { return 0.0; }

    // return slider value
    return (m_dialValue[a_index]);
}


////////////////////////////////////////////////////////////////////////////////
///
///  This method returns the minium range value of a dial.
///
///  \return 
///  Minimum range value.
///
////////////////////////////////////////////////////////////////////////////////
double cXTouchController::getDialRangeMin(int a_index)
{
    // sanity check
    if (!m_deviceReady || (!cContains(a_index, 0, getNumDials() - 1))) { return 0.0; }

    // return minimum range value
    return m_dialRange[a_index][0];
}


////////////////////////////////////////////////////////////////////////////////
///
///  This method returns the maximum range value of a dial.
///
///  \return 
///  Maximum range value.
///
////////////////////////////////////////////////////////////////////////////////
double cXTouchController::getDialRangeMax(int a_index)
{
    // sanity check
    if (!m_deviceReady || (!cContains(a_index, 0, getNumDials() - 1))) { return 0.0; }

    // return maximum range value
    return m_dialRange[a_index][1];
}


////////////////////////////////////////////////////////////////////////////////
///
///  This method assigns a range of values to the dial.
///
///  \param a_index  Index number of dial.
///  \param a_min    Minimum range value.
///  \param a_max    Maximum range value.
///
////////////////////////////////////////////////////////////////////////////////
void cXTouchController::setDialRange(int a_index, double a_min, double a_max)
{
    // sanity check
    if (!m_deviceReady || (!cContains(a_index, 0, getNumDials() - 1))) { return; }

    // check range values
    double min = cMin(a_min, a_max);
    double max = cMax(a_min, a_max);

    // set dial range
    m_dialRange[a_index][0] = min;
    m_dialRange[a_index][1] = max;
}


////////////////////////////////////////////////////////////////////////////////
///
///  This method sets a value to a dial.
///
///  \param a_index  Index number of dial.
///  \param a_value  Desired value.
///
////////////////////////////////////////////////////////////////////////////////
void cXTouchController::setDialValue(int a_index, double a_value)
{
    // sanity check
    if (!m_deviceReady || (!cContains(a_index, 0, getNumDials() - 1))) { return; }

    // compute midi value
    m_dialValue[a_index] = cClamp(a_value, m_dialRange[a_index][0], m_dialRange[a_index][1]);
    unsigned char value = convertTo(m_dialValue[a_index], m_dialRange[a_index][0], m_dialRange[a_index][1]);

    // x-touch mini
    if (m_model == C_XTOUCH_MINI)
    {
        if (cContains(a_index, 0, 7))
        {
            // setLayerA();
            sendMidi3(186, 1 + a_index, value);
        }
        else if (cContains(a_index, 8, 15))
        {
            // setLayerB();
            // sendMidi3(186, 11 + a_index, value);
        }
    }

    // x-touch compact
    else if (m_model == C_XTOUCH_COMPACT)
    {
        if (cContains(a_index, 0, 15))
        {
            setLayerA();
            sendMidi3(176, 10 + a_index, value);
        }
        else if (cContains(a_index, 16, 31))
        {
            setLayerB();
            sendMidi3(176, 37 + a_index, value);
        }
    }
}


////////////////////////////////////////////////////////////////////////////////
///
///  This method sets the desired display mode of a dial. (0, 1, 2, or 3)
///
///  \param a_index  Index number of dial.
///  \param a_mode   Desired mode.
///
////////////////////////////////////////////////////////////////////////////////
void cXTouchController::setDialMode(int a_index, int a_mode)
{
    // sanity check
    if (!m_deviceReady || (!cContains(a_index, 0, getNumDials() - 1))) { return; }

    // clamp mode value
    int mode = cClamp(a_mode, 0, 4);

    // x-touch mini
    if (m_model == C_XTOUCH_MINI)
    {
        if (cContains(a_index, 0, 7))
        {
            // setLayerA();
            sendMidi3(176, 1 + a_index, mode);
        }
        else if (cContains(a_index, 8, 15))
        {
            // setLayerB();
            // sendMidi3(176, a_index - 8, mode);
        }
    }

    // x-touch compact
    else if (m_model == C_XTOUCH_COMPACT)
    {
        if (cContains(a_index, 0, 15))
        {
            setLayerA();
            sendMidi3(177, 10 + a_index, mode);
        }
        else if (cContains(a_index, 16, 31))
        {
            setLayerB();
            sendMidi3(177, 10 + a_index, mode);
        }
    }
}


////////////////////////////////////////////////////////////////////////////////
///
///  This method sets the display mode of the dial to single.
///
///  \param a_index  Index number of dial.
///
////////////////////////////////////////////////////////////////////////////////
void cXTouchController::setDialModeSingle(int a_index)
{
    setDialMode(a_index, 0);
}


////////////////////////////////////////////////////////////////////////////////
///
///  This method sets the display mode of the dial to pan.
///
///  \param a_index  Index number of dial.
///
////////////////////////////////////////////////////////////////////////////////
void cXTouchController::setDialModePan(int a_index)
{
    setDialMode(a_index, 1);
}


////////////////////////////////////////////////////////////////////////////////
///
///  This method sets the display mode of the dial to fan.
///
///  \param a_index  Index number of dial.
///
////////////////////////////////////////////////////////////////////////////////
void cXTouchController::setDialModeFan(int a_index)
{
    setDialMode(a_index, 2);
}


////////////////////////////////////////////////////////////////////////////////
///
///  This method sets the display mode of the dial to spread.
///
///  \param a_index  Index number of dial.
///
////////////////////////////////////////////////////////////////////////////////
void cXTouchController::setDialModeSpread(int a_index)
{
    setDialMode(a_index, 3);
}


////////////////////////////////////////////////////////////////////////////////
///
///  This method sets the display mode of the dial to trim.
///
///  \param a_index  Index number of dial.
///
////////////////////////////////////////////////////////////////////////////////
void cXTouchController::setDialModeTrim(int a_index)
{
    setDialMode(a_index, 4);
}


////////////////////////////////////////////////////////////////////////////////
///
///  This method turns the lights of the dial ON.
///
///  \param a_index  Index number of dial.
///
////////////////////////////////////////////////////////////////////////////////
void cXTouchController::setDialLightOn(int a_index)
{
    // sanity check
    if (!m_deviceReady || (!cContains(a_index, 0, getNumDials() - 1))) { return; }

    // x-touch mini
    if (m_model == C_XTOUCH_MINI)
    {
    }
    // x-touch compact
    else if (m_model == C_XTOUCH_COMPACT)
    {
        if (cContains(a_index, 0, 38))
        {
            sendMidi3(144, a_index, 2);
        }
    }
}


////////////////////////////////////////////////////////////////////////////////
///
///  This method turns the lights of the dial OFF.
///
///  \param a_index  Index number of dial.
///
////////////////////////////////////////////////////////////////////////////////
void cXTouchController::setDialLightOff(int a_index)
{
    // sanity check
    if (!m_deviceReady || (!cContains(a_index, 0, getNumDials() - 1))) { return; }

    // x-touch mini
    if (m_model == C_XTOUCH_MINI)
    {
    }
    // x-touch compact
    else if (m_model == C_XTOUCH_COMPACT)
    {
        if (cContains(a_index, 0, 38))
        {
            sendMidi3(144, a_index, 0);
        }
    }
}


////////////////////////////////////////////////////////////////////////////////
///
///  This method turns the lights of the dial to blinking.
///
///  \param a_index  Index number of dial.
///
////////////////////////////////////////////////////////////////////////////////
void cXTouchController::setDialLightBlinking(int a_index)
{
    // sanity check
    if (!m_deviceReady || (!cContains(a_index, 0, getNumDials() - 1))) { return; }

    // x-touch mini
    if (m_model == C_XTOUCH_MINI)
    {
    }
    // x-touch compact
    else if (m_model == C_XTOUCH_COMPACT)
    {
        if (cContains(a_index, 0, 38))
        {
            sendMidi3(144, a_index, 2);
        }
    }
}


////////////////////////////////////////////////////////////////////////////////
///
///  This method sets a value to all dials.
///
///  \param a_value  Desired value
///
////////////////////////////////////////////////////////////////////////////////
void cXTouchController::setAllDialsValue(double a_value)
{
    for (int i = 0; i < getNumDials(); i++)
    {
        setDialValue(i, a_value);
    }
}


////////////////////////////////////////////////////////////////////////////////
///
///  This method assigns a range of values to the dial.
///
///  \param a_min  Minimum value of range.
///  \param a_max  Maximum value of range.
///
////////////////////////////////////////////////////////////////////////////////
void cXTouchController::setAllDialsRange(double a_min, double a_max)
{
    for (int i = 0; i < getNumDials(); i++)
    {
        setDialRange(i, a_min, a_max);
    }
}


////////////////////////////////////////////////////////////////////////////////
///
///  This method sets the mode of all dials.
///
///  \param a_mode  Mode.
///
////////////////////////////////////////////////////////////////////////////////
void cXTouchController::setAllDialsMode(int a_mode)
{

    for (int i = 0; i < getNumDials(); i++)
    {
        setDialMode(i, a_mode);
    }
}


////////////////////////////////////////////////////////////////////////////////
///
///  This method sets the display mode of all dials to single.
///
////////////////////////////////////////////////////////////////////////////////
void cXTouchController::setAllDialsModeSingle()
{
    for (int i = 0; i < getNumDials(); i++)
    {
        setDialModeSingle(i);
    }
}


////////////////////////////////////////////////////////////////////////////////
///
///  This method sets the display mode of all dials to pan.
///
////////////////////////////////////////////////////////////////////////////////
void cXTouchController::setAllDialsModePan()
{
    for (int i = 0; i < getNumDials(); i++)
    {
        setDialModePan(i);
    }
}


////////////////////////////////////////////////////////////////////////////////
///
///  This method sets the display mode of all dials to fan.
///
////////////////////////////////////////////////////////////////////////////////
void cXTouchController::setAllDialsModeFan()
{
    for (int i = 0; i < getNumDials(); i++)
    {
        setDialModeFan(i);
    }
}


////////////////////////////////////////////////////////////////////////////////
///
///  This method sets the display mode of all dials to spread.
///
////////////////////////////////////////////////////////////////////////////////
void cXTouchController::setAllDialsModeSpread()
{
    for (int i = 0; i < getNumDials(); i++)
    {
        setDialModeSpread(i);
    }
}


////////////////////////////////////////////////////////////////////////////////
///
///  This method sets the display mode of all dials to trim.
///
////////////////////////////////////////////////////////////////////////////////
void cXTouchController::setAllDialsModeTrim()
{
    for (int i = 0; i < getNumDials(); i++)
    {
        setDialModeTrim(i);
    }
}


////////////////////////////////////////////////////////////////////////////////
///
///  This method turns the lights of all dials ON.
///
////////////////////////////////////////////////////////////////////////////////
void cXTouchController::setAllDialsLightOn()
{
    for (int i = 0; i < getNumDials(); i++)
    {
        setDialLightOn(i);
    }
}


////////////////////////////////////////////////////////////////////////////////
///
///  This method turns the lights of all dials OFF.
///
////////////////////////////////////////////////////////////////////////////////
void cXTouchController::setAllDialsLightOff()
{
    for (int i = 0; i < getNumDials(); i++)
    {
        setDialLightOff(i);
    }
}


////////////////////////////////////////////////////////////////////////////////
///
///  This method turns the lights of all dials to blinking.
///
////////////////////////////////////////////////////////////////////////////////
void cXTouchController::setAllDialsLightBlinking()
{
    for (int i = 0; i < getNumDials(); i++)
    {
        setDialLightBlinking(i);
    }
}


////////////////////////////////////////////////////////////////////////////////
///
///  This method returns the number of buttons.
///
///  \return 
///  Number of available buttons
///
////////////////////////////////////////////////////////////////////////////////
int cXTouchController::getNumButtons()
{
    // sanity check
    if (!m_deviceReady) { return 0; }

    // x-touch mini
    if (m_model == C_XTOUCH_MINI)
    {
        return 48;
    }

    // x-touch compact
    else if (m_model == C_XTOUCH_COMPACT)
    {
        return 110;
    }

    // non supported device
    else
    {
        return 0;
    }
}



////////////////////////////////////////////////////////////////////////////////
///
///  This method return the current mode of a button.
///
///  \param a_index  Index value.
///
///  \return 
///  Button mode.
///
////////////////////////////////////////////////////////////////////////////////
int cXTouchController::getButtonMode(int a_index)
{
    // sanity check
    if (!m_deviceReady || (!cContains(a_index, 0, getNumButtons() - 1))) { return 0; }

    // return button mode
    return m_buttonMode[a_index];
}

 
////////////////////////////////////////////////////////////////////////////////
///
///  This method returns the state of a button.
///
///  \param a_index  Index value.
///
///  \return 
///  Button state.
///
////////////////////////////////////////////////////////////////////////////////
bool cXTouchController::getButtonState(int a_index)
{
    // sanity check
    if (!m_deviceReady || (!cContains(a_index, 0, getNumButtons() - 1))) { return false; }

    // get state
    bool state = m_buttonState[a_index];

    // flip state if button is of momentary type
    if (m_buttonMode[a_index] == 0)
    {
        m_buttonState[a_index] = false;
    }

    // return result
    return state;
}


////////////////////////////////////////////////////////////////////////////////
///
///  This method returns the light behavior of a button.
///
///  \param a_index  Index value.
///
///  \return 
///  Light behavior mode.
///
////////////////////////////////////////////////////////////////////////////////
int cXTouchController::getButtonLightBehavior(int a_index)
{
    // sanity check
    if (!m_deviceReady || (!cContains(a_index, 0, getNumButtons() - 1))) { return 0; }
    
    // return light behavior
    return m_buttonBehavior[a_index];
}

 
////////////////////////////////////////////////////////////////////////////////
///
///  This method sets a button to toggle mode.
///
///  \param a_index  Index value.
///
////////////////////////////////////////////////////////////////////////////////
void cXTouchController::setButtonToggleMode(int a_index)
{
    // sanity check
    if (!m_deviceReady || (!cContains(a_index, 0, getNumButtons() - 1))) { return; }

    // enable toggle mode
    m_buttonMode[a_index] = 1;
}


////////////////////////////////////////////////////////////////////////////////
///
///  This method  sets a button to momentary mode.
///
///  \param a_index  Index value.
///
////////////////////////////////////////////////////////////////////////////////
void cXTouchController::setButtonMomentaryMode(int a_index)
{
    // sanity check
    if (!m_deviceReady || (!cContains(a_index, 0, getNumButtons() - 1))) { return; }

    // enable momentary mode
    m_buttonMode[a_index] = 0;
}


////////////////////////////////////////////////////////////////////////////////
///
///  This method sets the state of a button.
///
///  \param a_index  Index value.
///  \param a_state  Button state value.
///
////////////////////////////////////////////////////////////////////////////////
void cXTouchController::setButtonState(int a_index, bool a_state)
{
    // sanity check
    if (!m_deviceReady || (!cContains(a_index, 0, getNumButtons() - 1))) { return; }

    // set state
    m_buttonState[a_index] = a_state;

    // update light
    if (m_buttonMode[a_index] == 1)
    {
        if (a_state == true)
        {
            setButtonLightOn(a_index-8);
        }
        else
        {
            setButtonLightOff(a_index-8);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
///
///  This method sets a button to a given mode.
///
///  \param a_index  Index value.
///  \param a_mode   Mode
///
////////////////////////////////////////////////////////////////////////////////
void cXTouchController::setButtonMode(int a_index, int a_mode)
{
    // sanity check
    if (!m_deviceReady || (!cContains(a_index, 0, getNumButtons() - 1))) { return; }

    // clamp and assign new button mode
    m_buttonMode[a_index] = cClamp(a_mode, 0, 1);
}


////////////////////////////////////////////////////////////////////////////////
///
///  This method toggles the buttons ON.
///
///  \param a_index  Index value.
///
////////////////////////////////////////////////////////////////////////////////
void cXTouchController::setButtonToggleOn(int a_index)
{
    // sanity check
    if (!m_deviceReady || (!cContains(a_index, 0, getNumButtons() - 1))) { return; }

    // x-touch mini
    if (m_model == C_XTOUCH_MINI)
    {
    }
    // x-touch compact
    else if (m_model == C_XTOUCH_COMPACT)
    {
    }
}


////////////////////////////////////////////////////////////////////////////////
///
///  This method toggles the button OFF.
///
///  \param a_index  Index value.
///
////////////////////////////////////////////////////////////////////////////////
void cXTouchController::setButtonToggleOff(int a_index)
{
    // sanity check
    if (!m_deviceReady || (!cContains(a_index, 0, getNumButtons() - 1))) { return; }

    // x-touch mini
    if (m_model == C_XTOUCH_MINI)
    {
    }
    // x-touch compact
    else if (m_model == C_XTOUCH_COMPACT)
    {
    }
}


////////////////////////////////////////////////////////////////////////////////
///
///  This method sets the light mode of a button.
///
///  \param a_index  Index value.
///  \param a_mode   Mode
///
////////////////////////////////////////////////////////////////////////////////
void cXTouchController::setButtonLightBehavior(int a_index, int a_mode)
{
    // sanity check
    if (!m_deviceReady || (!cContains(a_index, 0, getNumButtons() - 1))) { return; }

    // x-touch mini
    if (m_model == C_XTOUCH_MINI)
    {
    }
    // x-touch compact
    else if (m_model == C_XTOUCH_COMPACT)
    {
    }
}


////////////////////////////////////////////////////////////////////////////////
///
///  This method sets the light of a button ON.
///
///  \param a_index  Index value.
///
////////////////////////////////////////////////////////////////////////////////
void cXTouchController::setButtonLightOn(int a_index)
{
    // sanity check
    if (!m_deviceReady || (!cContains(a_index, 0, getNumButtons() - 1))) { return; }

    // x-touch mini
    if (m_model == C_XTOUCH_MINI)
    {
        if (cContains(a_index, 0, 15))
        {
            //setLayerA();
            sendMidi3(144, a_index, 1);
        }
        if (cContains(a_index, 16, 31))
        {
            //setLayerB();
            //sendMidi3(144, a_index, 1);
        }
    }
    // x-touch compact
    else if (m_model == C_XTOUCH_COMPACT)
    {
        if (cContains(a_index, 0, 38))
        {
            setLayerA();
            sendMidi3(144, 16 + a_index, 1);
        }
        if (cContains(a_index, 39, 77))
        {
            setLayerB();
            sendMidi3(144, 32 + a_index, 1);
        }
    }
}


////////////////////////////////////////////////////////////////////////////////
///
///  This method sets the light of a button OFF.
///
///  \param a_index  Index value.
///
////////////////////////////////////////////////////////////////////////////////
void cXTouchController::setButtonLightOff(int a_index)
{
    // sanity check
    if (!m_deviceReady || (!cContains(a_index, 0, getNumButtons() - 1))) { return; }

    // x-touch mini
    if (m_model == C_XTOUCH_MINI)
    {
        if (cContains(a_index, 0, 15))
        {
            // setLayerA();
            sendMidi3(144, a_index, 0);
        }
        if (cContains(a_index, 16, 31))
        {
            // setLayerB();
            sendMidi3(144, a_index -16, 0);
        }
    }
    // x-touch compact
    else if (m_model == C_XTOUCH_COMPACT)
    {
        if (cContains(a_index, 0, 38))
        {
            setLayerA();
            sendMidi3(144, 16 + a_index, 0);
        }
        if (cContains(a_index, 39, 77))
        {
            setLayerB();
            sendMidi3(144, 32 + a_index, 0);
        }
    }
}

 
////////////////////////////////////////////////////////////////////////////////
///
///  This method sets the light of a button to blinking.
///
///  \param a_index  Index value.
///
////////////////////////////////////////////////////////////////////////////////
void cXTouchController::setButtonLightBlinking(int a_index)
{
    // sanity check
    if (!m_deviceReady || (!cContains(a_index, 0, getNumButtons() - 1))) { return; }

    // x-touch mini
    if (m_model == C_XTOUCH_MINI)
    {
        if (cContains(a_index, 0, 15))
        {
            //setLayerA();
            sendMidi3(144, a_index, 2);
        }
        if (cContains(a_index, 16, 31))
        {
            //setLayerB();
            sendMidi3(144, a_index - 16, 2);
        }
    }
    // x-touch compact
    else if (m_model == C_XTOUCH_COMPACT)
    {
        if (cContains(a_index, 0, 38))
        {
            setLayerA();
            sendMidi3(144, 16 + a_index, 2);
        }
        if (cContains(a_index, 39, 77))
        {
            setLayerB();
            sendMidi3(144, 32 + a_index, 2);
        }
    }
}


////////////////////////////////////////////////////////////////////////////////
///
///  This method sets the state of all buttons
///
///  \param a_state  Button state value.
///
////////////////////////////////////////////////////////////////////////////////
void cXTouchController::setAllButtonsState(bool a_state)
{
    for (int i = 0; i < getNumButtons(); i++)
    {
        setButtonState(i, a_state);
    }
}

 
////////////////////////////////////////////////////////////////////////////////
///
///  This method sets all buttons to toggle mode.
///
////////////////////////////////////////////////////////////////////////////////
void cXTouchController::setAllButtonsToggleMode()
{
    for (int i = 0; i < getNumButtons(); i++)
    {
        setButtonToggleMode(i);
    }
}

 
////////////////////////////////////////////////////////////////////////////////
///
///  This method sets all buttons to momentary mode.
///
////////////////////////////////////////////////////////////////////////////////
void cXTouchController::setAllButtonsMomentaryMode()
{
    for (int i = 0; i < getNumButtons(); i++)
    {
        setButtonMomentaryMode(i);
    }
}


////////////////////////////////////////////////////////////////////////////////
///
///  This method sets all buttons to a given mode. 
///
///  \param a_mode  Button mode.
///
////////////////////////////////////////////////////////////////////////////////
void cXTouchController::setAllButtonsMode(int a_mode)
{
    for (int i = 0; i < getNumButtons(); i++)
    {
        setButtonMode(i, a_mode);
    }
}


////////////////////////////////////////////////////////////////////////////////
///
///  This method toggles all buttons ON.
///
////////////////////////////////////////////////////////////////////////////////
void cXTouchController::setAllButtonsToggleOn()
{
    for (int i = 0; i < getNumButtons(); i++)
    {
        setButtonToggleOn(i);
    }
}


////////////////////////////////////////////////////////////////////////////////
///
///  This method toggles all buttons OFF.
///
////////////////////////////////////////////////////////////////////////////////
void cXTouchController::setAllButtonsToggleOff()
{
    for (int i = 0; i < getNumButtons(); i++)
    {
        setButtonToggleOff(i);
    }
}


////////////////////////////////////////////////////////////////////////////////
///
///  This method sets the light behavior of all buttons.
///
///  \param a_mode  Light behavior mode.
///
////////////////////////////////////////////////////////////////////////////////
void cXTouchController::setAllButtonsLightBehavior(int a_mode)
{
    for (int i = 0; i < getNumButtons(); i++)
    {
        setButtonLightBehavior(i, a_mode);
    }
}


////////////////////////////////////////////////////////////////////////////////
///
///  This method sets the light of all buttons ON.
///
////////////////////////////////////////////////////////////////////////////////
void cXTouchController::setAllButtonsLightOn()
{
    for (int i = 0; i < getNumButtons(); i++)
    {
        setButtonLightOn(i);
    }
}


////////////////////////////////////////////////////////////////////////////////
///
///  This method sets the light of all buttons OFF.
///
////////////////////////////////////////////////////////////////////////////////
void cXTouchController::setAllButtonsLightOff()
{
    for (int i = 0; i < getNumButtons(); i++)
    {
        setButtonLightOff(i);
    }
}


////////////////////////////////////////////////////////////////////////////////
///
///  This method sets the light of all buttona to blinking.
///
////////////////////////////////////////////////////////////////////////////////
void cXTouchController::setAllButtonsLightBlinking()
{
    for (int i = 0; i < getNumButtons(); i++)
    {
        setButtonLightBlinking(i);
    }
}


////////////////////////////////////////////////////////////////////////////////
///
///  This method implements a call back for incoming data from XTouchInterface.
///
///  \param  a_deltatime  Timestamp.
///  \param  a_message    Incoming midi message.
///
////////////////////////////////////////////////////////////////////////////////
void cXTouchController::callbackMidiIn(double a_deltatime, std::vector< unsigned char > *a_message)
{
    // get number of bytes
    size_t nBytes = a_message->size();

    // sanity check
    if (nBytes != 3) { return; }

    // get bytes
    int value0 = (int)a_message->at(0);
    int value1 = (int)a_message->at(1);
    int value2 = (int)a_message->at(2);
    
    // variables
    int index;


    ////////////////////////////////////////////////////////////////////////////
    // X-TOUCH COMPACT
    ////////////////////////////////////////////////////////////////////////////

    if (m_model == C_XTOUCH_COMPACT)
    {
        ////////////////////////////////////////////////////////////////////////
        // SLIDERS
        ////////////////////////////////////////////////////////////////////////

        if (value0 == 176)
        {
            if (cContains(value1, 1, 9))
            {
                index = value1 - 1;
                m_sliderValue[index] = convertFrom(value2, m_sliderRange[index][0], m_sliderRange[index][1]);
            }
            else if (cContains(value1, 28, 36))
            {
                index = value1 - 19;
                m_sliderValue[index] = convertFrom(value2, m_sliderRange[index][0], m_sliderRange[index][1]);
            }
        }

        ////////////////////////////////////////////////////////////////////////
        // DIALS
        ////////////////////////////////////////////////////////////////////////

        if (value0 == 176)
        {
            if (cContains(value1, 10, 25))
            {
                index = value1 - 10;
                m_dialValue[index] = convertFrom(value2, m_dialRange[index][0], m_dialRange[index][1]);
            }
            else if (cContains(value1, 37, 52))
            {
                index = value1 - 21;
                m_dialValue[index] = convertFrom(value2, m_dialRange[index][0], m_dialRange[index][1]);
            }
        }

        ////////////////////////////////////////////////////////////////////////
        // BUTTONS
        ////////////////////////////////////////////////////////////////////////

        // button down
        if (value0 == 144)
        {
            if (cContains(value1, 0, 109))
            {
                index = value1;

                // momentary button
                if (m_buttonMode[index] == 0)
                {
                    m_buttonState[index] = true;
                }

                // toggle button
                else if (m_buttonMode[index] == 1)
                {
                    m_buttonState[index] = !m_buttonState[index];
                }
            }
        }

        // button up
        if (value0 == 128)
        {
            if (cContains(value1, 0, 109))
            {
                index = value1;

                // toggle button
                if (m_buttonMode[index] == 1)
                {
                    int lightIndex = -1;

                    if (cContains(value1, 16, 54))
                    {
                        lightIndex = index - 16;
                    }
                    else if (cContains(value1, 71, 109))
                    {
                        lightIndex = index - 32;
                    }
                    if (lightIndex != -1)
                    {
                        if (m_buttonState[index])
                        {
                            setButtonLightOn(lightIndex);
                        }
                        else
                        {
                            setButtonLightOff(lightIndex);
                        }
                    }
                }
            }
        }

    }


    ////////////////////////////////////////////////////////////////////////////
    // X-TOUCH MINI
    ////////////////////////////////////////////////////////////////////////////

    else if (m_model == C_XTOUCH_MINI)
    {
        ////////////////////////////////////////////////////////////////////////
        // SLIDERS
        ////////////////////////////////////////////////////////////////////////

        if (value0 == 186)
        {
            if (cContains(value1, 9, 10))
            {
                index = value1 - 9;
                m_sliderValue[index] = convertFrom(value2, m_sliderRange[index][0], m_sliderRange[index][1]);
            }
        }

        ////////////////////////////////////////////////////////////////////////
        // DIALS
        ////////////////////////////////////////////////////////////////////////

        if (value0 == 186)
        {
            if (cContains(value1, 1, 8))
            {
                index = value1 - 1;
                m_dialValue[index] = convertFrom(value2, m_dialRange[index][0], m_dialRange[index][1]);
            }
            else if (cContains(value1, 11, 18))
            {
                index = value1 - 3;
                m_dialValue[index] = convertFrom(value2, m_dialRange[index][0], m_dialRange[index][1]);
            }
        }

        ////////////////////////////////////////////////////////////////////////
        // BUTTONS
        ////////////////////////////////////////////////////////////////////////

        // button down
        if (value0 == 154)
        {
            if (cContains(value1, 0, 47))
            {
                index = value1;

                // momentary button
                if (m_buttonMode[index] == 0)
                {
                    m_buttonState[index] = true;
                }

                // toggle button
                else if (m_buttonMode[index] == 1)
                {
                    m_buttonState[index] = !m_buttonState[index];
                }
            }
        }

        // button up
        if (value0 == 138)
        {
            if (cContains(value1, 0, 47))
            {
                index = value1;

                // toggle button
                if (m_buttonMode[index] == 1)
                {
                    int lightIndex = -1;

                    if (cContains(value1, 8, 23))
                    {
                        lightIndex = index - 8;
                    }
                    else if (cContains(value1, 32, 47))
                    {
                        lightIndex = index - 16;
                    }
                    if (lightIndex != -1)
                    {
                        if (m_buttonState[index])
                        {
                            setButtonLightOn(lightIndex);
                        }
                        else
                        {
                            setButtonLightOff(lightIndex);
                        }
                    }
                }
            }
        }
    }

#if 0

    ////////////////////////////////////////////////////////////////////////////
    // DEBUG DATA
    ////////////////////////////////////////////////////////////////////////////

  for (unsigned int i = 0; i<nBytes; i++)
    std::cout << "Byte " << i << " = " << (int)a_message->at(i) << ", ";
  if (nBytes > 0)
      std::cout << "stamp = " << a_deltatime << ", ";
  if (nBytes > 0)
      std::cout << "port in = " << m_portNumberIn << "  port out = " << m_portNumberOut << std::endl;

#endif

}


////////////////////////////////////////////////////////////////////////////////
///
///  This method sends a two-byte command through the midi port
///
///  \param a_data0  Data value 0
///  \param a_data1  Data value 1
///
////////////////////////////////////////////////////////////////////////////////
void cXTouchController::sendMidi2(unsigned char a_data0, unsigned char a_data1)
{
    // check if system is ready
    if (!m_deviceReady) { return; }

    // create midi message
    std::vector<unsigned char> message;
    message.push_back(a_data0);
    message.push_back(a_data1);

    // send midi message
    ((RtMidiOut*)(midiout))->sendMessage(&message);
}


////////////////////////////////////////////////////////////////////////////////
///
///  This method sends a three-byte command through the midi port
///
///  \param a_data0  Data value 0
///  \param a_data1  Data value 1
///  \param a_data2  Data value 2
///
////////////////////////////////////////////////////////////////////////////////
void cXTouchController::sendMidi3(unsigned char a_data0, unsigned char a_data1, unsigned char a_data2)
{
    // check if system is ready
    if (!m_deviceReady) { return; }

    // create midi message
    std::vector<unsigned char> message;
    message.push_back(a_data0);
    message.push_back(a_data1);
    message.push_back(a_data2);

    // send midi message
    ((RtMidiOut*)(midiout))->sendMessage(&message);
}


////////////////////////////////////////////////////////////////////////////////
///
///  This method converts a midi value (0-127) into a ranged value.
///
///  \param a_value     Input value from midi interface.
///  \param a_rangeMin  Minimum range value after conversion.
///  \param a_rangeMax  Maximum range value after conversion.
///
///  \return 
///  Converted value.
///
////////////////////////////////////////////////////////////////////////////////
double cXTouchController::convertFrom(unsigned char a_value, double a_rangeMin, double a_rangeMax)
{
    // check range values
    double min = cMin(a_rangeMin, a_rangeMax);
    double max = cMax(a_rangeMin, a_rangeMax);

    // handle case when min-max values are equal
    if (min == max) { return min; }

    // clamp value
    unsigned char value = cClamp(a_value, (unsigned char)0, (unsigned char)127);

    // convert value
    double convertedValue = ((double)value / 127.0) * (max - min) + min;

    // clamp converted value
    double result = cClamp(convertedValue, min, max);

    // return result
    return result;
}


////////////////////////////////////////////////////////////////////////////////
///
///  This method converts a ranged value into a midi value (0-127).
///
///  \param a_value     Input value.
///  \param a_rangeMin  Minimum range value before conversion.
///  \param a_rangeMax  Maximum range value before conversion.
///
///  \return 
///  Converted value.
///
////////////////////////////////////////////////////////////////////////////////
unsigned char cXTouchController::convertTo(double a_value, double a_rangeMin, double a_rangeMax)
{
    // check range values
    double min = cMin(a_rangeMin, a_rangeMax);
    double max = cMax(a_rangeMin, a_rangeMax);

    // handle case when min-max values are equal
    if (min == max) { return 0; }

    // clamp value
    double value = cClamp(a_value, min, max);

    // convert value
    unsigned char convertedValue = (unsigned char)(127.0 * (value - min) / (max - min));

    // clamp converted value
    unsigned char result = cClamp(convertedValue, (unsigned char)0, (unsigned char)127);

    // return result
    return result;
}


} // namespace chai3d
