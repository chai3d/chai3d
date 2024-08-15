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


#ifndef __CXTOUCHCONTROLLER_H_
#define __CXTOUCHCONTROLLER_H_

// related header files
#include "devices/CGenericDevice.h"

// chai3d namespace
namespace chai3d {


////////////////////////////////////////////////////////////////////////////////
///
///  \file       CXTouchController.h
///
///  \brief 
///  Implementation of a interface for the X-Touch midi controller.
///
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
///
///    Defines the list of devices currently supported by CHAI3D. 
///
////////////////////////////////////////////////////////////////////////////////
enum cXTouchModel
{
    C_XTOUCH_NOT_DETECTED,
    C_XTOUCH_COMPACT,
    C_XTOUCH_MINI
};

const int C_XTOUCH_MAX_BUTTONS = 200;
const int C_XTOUCH_MAX_SLIDERS = 200;
const int C_XTOUCH_MAX_DIALS   = 200;



////////////////////////////////////////////////////////////////////////////////
///
///  \class      cXTouchController
///  \ingroup    devices
///
///  \brief
///  This class implements support for the X-Touch midi controller.
///
///  \details
///  This class implements support for the X-Touch midi controller.
///
////////////////////////////////////////////////////////////////////////////////

class cXTouchController : public cGenericDevice
{  
    ////////////////////////////////////////////////////////////////////////////
    // CONSTRUCTOR & DESTRUCTOR:
    ////////////////////////////////////////////////////////////////////////////

public:

    //! Constructor of cXTouchController.
    cXTouchController(unsigned int a_deviceNumber = 0);

    //! Destructor of cXTouchController.
    virtual ~cXTouchController();


    ////////////////////////////////////////////////////////////////////////////
    // PUBLIC STATIC METHODS:
    ////////////////////////////////////////////////////////////////////////////

public:

    //! This method returns the number of haptic devices available for this class of devices.
    static unsigned int getNumDevices();


    ////////////////////////////////////////////////////////////////////////////
    // PUBLIC METHODS - GENERAL
    ////////////////////////////////////////////////////////////////////////////

public:

    //! This method opens a connection to this device.
    virtual bool open();

    //! This method closes the connection to this device.
    virtual bool close();

    //! This method returns the model of X-Touch interface.
    cXTouchModel getModel() { return m_model; }

    //! This method enables layer A
    void setLayerA() { setLayer(0); }

    //! This method enables layer B
    void setLayerB() { setLayer(1); }

    //! This method selects the desired layer (A or B)
    void setLayer(int a_layer);

    //! This method iniaitalizes the table to its default state.
    void initialize();


    ////////////////////////////////////////////////////////////////////////////
    // PUBLIC METHODS - SLIDERS
    ////////////////////////////////////////////////////////////////////////////

public:

    //! This method returns the number of active sliders.
    int getNumSliders();

    //! This method returns the value of a slider.
    double getSliderValue(int a_index);

    //! This method sets the value of a slider.
    void setSliderValue(int a_index, double a_value);

    //! This method sets the value of a all sliders.
    void setAllSlidersValue(double a_value);


    ////////////////////////////////////////////////////////////////////////////
    // PUBLIC METHODS - DIALS
    ////////////////////////////////////////////////////////////////////////////

public:

    //! This method returns the number of dials.
    int getNumDials();

    //! This method returns the value of a dial.
    double getDialValue(int a_index);

    //! This method returns the minium range value of a dial.
    double getDialRangeMin(int a_index);

    //! This method returns the maximum range value of a dial.
    double getDialRangeMax(int a_index);

    //! This method assigns a range of values to the dial.
    void setDialRange(int a_index, double a_min, double a_max);

    //! This method sets a value to a dial.
    void setDialValue(int a_index, double a_value);

    //! This method sets the mode of a dial.
    void setDialMode(int a_index, int a_mode);

    //! This method sets the display mode of the dial to single.
    void setDialModeSingle(int a_index);

    //!  This method sets the display mode of the dial to pan.
    void setDialModePan(int a_index);

    //! This method sets the display mode of the dial to fan.
    void setDialModeFan(int a_index);

    //! This method sets the display mode of the dial to spread.
    void setDialModeSpread(int a_index);

    //! This method sets the display mode of the dial to trim.
    void setDialModeTrim(int a_index);

    //! This method turns the lights of the dial ON.
    void setDialLightOn(int a_index);

    //! This method turns the lights of the dial OFF.
    void setDialLightOff(int a_index);

    //! This method turns the lights of the dial to blinking.
    void setDialLightBlinking(int a_index);

    //! This method sets a value to all dials.
    void setAllDialsValue(double a_value);

    //! This method assigns a range of values to the dial.
    void setAllDialsRange(double a_min, double a_max);

    //! This method sets the mode of all dials.
    void setAllDialsMode(int a_mode);

    //! This method sets the display mode of all dials to single.
    void setAllDialsModeSingle();

    //!  This method sets the display mode of all dials to pan.
    void setAllDialsModePan();

    //! This method sets the display mode of all dials to fan.
    void setAllDialsModeFan();
    
    //! This method sets the display mode of all dials to spread.
    void setAllDialsModeSpread();

    //! This method sets the display mode of all dials to trim.
    void setAllDialsModeTrim();

    //! This method turns the lights of all dials ON.
    void setAllDialsLightOn();

    //! This method turns the lights of all dials OFF.
    void setAllDialsLightOff();

    //! This method turns the lights of all dials to blinking.
    void setAllDialsLightBlinking();


    ////////////////////////////////////////////////////////////////////////////
    // PUBLIC METHODS - BUTTONS
    ////////////////////////////////////////////////////////////////////////////

public:

    //! This method returns the number of buttons.
    int getNumButtons();

    //! This method return the current mode of a button.
    int getButtonMode(int a_index);

    //! This method returns the state of a button.
    bool getButtonState(int a_index);

    //! This method returns the light behavior of a button.
    int getButtonLightBehavior(int a_index);

     //! This method sets the state of a button.
    void setButtonState(int a_index, bool a_state);

    //! This method sets a button to toggle mode.
    void setButtonToggleMode(int a_index);

    //! This method  sets a button to momentary mode.
    void setButtonMomentaryMode(int a_index);

    //! This method set a button to a given mode.
    void setButtonMode(int a_index, int a_mode);

    //! This method toggles the buttons ON.
    void setButtonToggleOn(int a_index);

    //! This method toggles the button OFF.
    void setButtonToggleOff(int a_index);

    //! This method sets the light mode of a button.
    void setButtonLightBehavior(int a_index, int a_mode);

    //! This method sets the light of a button ON.
    void setButtonLightOn(int a_index);

    //! This method sets the light of a button OFF.
    void setButtonLightOff(int a_index);

    //! This method sets the light of a button to blinking.
    void setButtonLightBlinking(int a_index);

    //! This method sets the state of all buttons.
    void setAllButtonsState(bool a_state);

    //! This method sets all buttons to toggle mode.
    void setAllButtonsToggleMode();

    //! This method sets all buttons to momentary mode.
    void setAllButtonsMomentaryMode();

    //! This method sets all buttons to a given mode. 
    void setAllButtonsMode(int a_mode);

    //! This method toggles all buttons ON.
    void setAllButtonsToggleOn();

    //! This method toggles all buttons OFF.
    void setAllButtonsToggleOff();

    //! This method sets the light behavior of all buttons.
    void setAllButtonsLightBehavior(int a_mode);

    //! This method sets the light of all buttons ON.
    void setAllButtonsLightOn();

    //! This method sets the light of all buttons OFF.
    void setAllButtonsLightOff();

    //! This method sets the light of all buttona to blinking.
    void setAllButtonsLightBlinking();


    ////////////////////////////////////////////////////////////////////////////
    // PUBLIC METHODS:
    ////////////////////////////////////////////////////////////////////////////

public:

    //! This method is a callback for all midi data coming from the interface.
    void callbackMidiIn(double a_deltatime, std::vector< unsigned char > *a_message);


    ////////////////////////////////////////////////////////////////////////////
    // PROTECTED METHODS:
    ////////////////////////////////////////////////////////////////////////////

public:

    //! This method sends a message through the midi port composed of three bytes.
    void sendMidi3(unsigned char a_data0, unsigned char a_data1, unsigned char a_data2);

    //! This method sends a message through the midi port composed of two bytes.
    void sendMidi2(unsigned char a_data0, unsigned char a_data1);

    //! This method converts a midi value (0-127) into a ranged value.
    double convertFrom(unsigned char a_value, double a_rangeMin, double a_rangeMax);

    //! This method converts a ranged value into a midi value (0-127).
    unsigned char convertTo(double a_value, double a_rangeMin, double a_rangeMax);


    ////////////////////////////////////////////////////////////////////////////
    // PROTECTED MEMBERS:
    ////////////////////////////////////////////////////////////////////////////

protected:

    //! Device model.
    cXTouchModel m_model;

    //! Midi interface input data.
    void *midiin;

    //! Midi interface output data.
    void *midiout;

    //! Port number MIDI in
    unsigned int m_portNumberIn;

    //! Port number MIDI out
    unsigned int m_portNumberOut;

    //! Current layer.
    int m_layer;

    //! Slider values.
    double m_sliderValue[C_XTOUCH_MAX_SLIDERS];

    //! Slider Ranges.
    double m_sliderRange[C_XTOUCH_MAX_DIALS][2];

    //! Dial values.
    double m_dialValue[C_XTOUCH_MAX_DIALS];

    //! Dial Ranges.
    double m_dialRange[C_XTOUCH_MAX_DIALS][2];

    //! Button states.
    bool m_buttonState[C_XTOUCH_MAX_BUTTONS];

    //! Button modes.
    int m_buttonMode[C_XTOUCH_MAX_BUTTONS];

    //! Button light behavior.
    int m_buttonBehavior[C_XTOUCH_MAX_BUTTONS];
};


} // chai3d namespace


#endif // __CXTOUCHCONTROLLER_H_

