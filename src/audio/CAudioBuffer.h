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
#ifndef CAudioBufferH
#define CAudioBufferH
//------------------------------------------------------------------------------
#include "math/CMaths.h"
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
namespace chai3d {
//------------------------------------------------------------------------------

//==============================================================================
/*!
    \file       CAudioBuffer.h

    \brief 
    Implements an audio Buffer.
*/
//==============================================================================

//==============================================================================
/*!
    \class      cAudioBuffer
    \ingroup    audio

    \brief
    This class implements an audio buffer.

    \details
    This class implements an audio buffer. An audio buffer contains audio
    data and information that describes the specifications of the sound
    signal such as frequency, length, and format.
*/
//==============================================================================
class cAudioBuffer
{    
    //--------------------------------------------------------------------------
    // CONSTRUCTOR & DESTRUCTOR:
    //--------------------------------------------------------------------------

public:

    //! Constructor of cAudioBuffer.
    cAudioBuffer();

    //! Destructor of cAudioBuffer.
    virtual ~cAudioBuffer();


    //--------------------------------------------------------------------------
    // PUBLIC METHODS:
    //--------------------------------------------------------------------------

public:

    //! This method clears the audio buffer.
    void clear() { cleanup(); }

    //! This method sets a buffer by passing a pointer to the audio data and defines the data specifications.
    bool setup(unsigned char* a_data, const unsigned int a_size, int a_frequency, bool a_stereo, unsigned short a_bitsPerSample);

    //! This method loads an audio file by passing the filename as argument.
    bool loadFromFile(const std::string& a_filename);

    //! This method returns the filename from which this audio data was most recently loaded.
    std::string getFilename() const { return (m_filename); }

    //! This method returns the OpenAL buffer ID.
    unsigned int getBuffer() { return (m_buffer); }

    //! This method returns the size in bytes of the audio data.
    unsigned int getSizeInBytes() { return (m_sizeInBytes); }

    //! This method returns the number of samples that compose the audio data.
    unsigned int getNumSamples() { return m_numSamples; }

    //! This method returns the duration in seconds of the audio data.
    double getDuration() { return (m_duration); }

    //! This method returns the sampling rate of the audio data.
    double getSamplingRate() { return (m_samplingRate); }

    //! This method returns __true__ if the audio data in in __stereo__ format, otherwise __false__.
    bool getStereo() { return (m_stereo); }

    //! This method returns the sample format of the audio data.
    int getBitsPerSample() { return (m_bitsPerSample); }

    //! This method returns a pointer to the audio data.
    unsigned char* getData() { return (m_data); }

    //! This method returns a sample from the left channel by passing time as argument
    short getSampleByTimeL(const double a_time, const bool a_loop);

    //! This method returns a sample from the right channel by passing time as argument
    short getSampleByTimeR(const double a_time, const bool a_loop);

    //! This method returns the current sample on the left channel.
    short getSampleByIndexL(const unsigned int a_index, const bool a_loop);

    //! This method returns the current sample on the right channel.
    short getSampleByIndexR(const unsigned int a_index, const bool a_loop);

    //! This method converts a __stereo__ stream to __mono__.
    bool convertToMono();


    //--------------------------------------------------------------------------
    // PROTECTED METHODS:
    //--------------------------------------------------------------------------

protected:

    //! This methods checks for any OpenAL errors.
    bool checkError();

    //! This method clears all memory.
    bool cleanup();

    //! This method computes the number of samples and duration stored in the audio buffer.
    void computeNumSamplesAndDuration();


    //--------------------------------------------------------------------------
    // PROTECTED MEMBERS:
    //--------------------------------------------------------------------------

protected:

    //! Audio filename.
    std::string m_filename;

    //! Audio data.
    unsigned char* m_data;
    
    //! OpenAL buffer ID.
    unsigned int m_buffer;

    //! Audio buffer size in bytes.
    unsigned int m_sizeInBytes;

    //! Number of audio samples in audio buffer.
    unsigned int m_numSamples;

    //! Duration of the audio signal stored in the audio buffer
    double m_duration;

    //! Sampling rate of the audio signal.
    double m_samplingRate;

    //! Audio data format (__stereo__ = __true__, __mono__ = __false__).
    bool m_stereo;

    //! Audio data resolution. Number of bits per sample (8 or 16).
    unsigned short m_bitsPerSample;

    //! Audio data ownership flag
    bool m_flagDeleteData;
};

//------------------------------------------------------------------------------
} // namespace chai3d
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
#endif
//------------------------------------------------------------------------------
