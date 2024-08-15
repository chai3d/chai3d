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
    \author    Sebastien Grange
    \version   3.3.0
*/
//==============================================================================

//------------------------------------------------------------------------------
#include "files/CFileAudioMP3.h"
//------------------------------------------------------------------------------
#include "math/CConstants.h"
//------------------------------------------------------------------------------
#define MINIMP3_IMPLEMENTATION
#include "minimp3.h"
#include <sys/stat.h>
//------------------------------------------------------------------------------
using namespace std;
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
using namespace chai3d;
namespace chai3d {
//------------------------------------------------------------------------------

//==============================================================================
/*!
    This function loads an audio MP3 file into memory. \n
    If the operation succeeds, then the functions returns __true__ and the 
    audio data is loaded into memory. The audio data, size, frequency and format are
    returned by argument. \n
    If the operation fails, then the function returns __false__. 

    \param  a_filename       Filename.
    \param  a_data           Returned pointer to audio data.
    \param  a_sizeInBytes    Returned size in bytes of the audio data.
    \param  a_bitsPerSample  Returned number of bits per sample (8 or 16).
    \param  a_samplingRate   Returned sampling wait of audio date.
    \param  a_numSamples     Returned number of audio samples.
    \param  a_stereo         Returns __true__ if stereo, __false__ otherwise.

    \return __true__ in case of success, __false__ otherwise.
*/
//==============================================================================
bool cLoadFileMP3(const std::string& a_filename, 
    unsigned char*& a_data,
    unsigned int* a_sizeInBytes,
    unsigned short* a_bitsPerSample,
    double* a_samplingRate,
    unsigned int* a_numSamples,
    bool* a_stereo)
{
    struct stat st;
    unsigned char* file_buf = 0;
    int music_size = 0;
    int alloc_samples = 1024 * 1024;
    int num_samples = 0;
    int16_t* music_buf = (int16_t*)malloc(alloc_samples * 2 * 2);

    // open mp3 file
    FILE* fd = fopen(a_filename.c_str(), "rb");
    if (fd == 0)
    {
        return C_ERROR;
    }

    // retrieve information about file
    if (fstat(fileno(fd), &st) < 0)
    {
        fclose(fd);
        return C_ERROR;
    }

    // allocate memory and load file content
    file_buf = (unsigned char*)malloc(st.st_size + (off_t)(1));
    if (file_buf != NULL)
    {
        if (fread(file_buf, st.st_size, 1, fd) < 1)
        {
            fclose(fd);
            return C_ERROR;
        }
        file_buf[st.st_size] = 0;
    }
    else
    {
        return C_ERROR;
    }

    // set file size
    music_size = st.st_size;

    // parse mp3 file
    if (file_buf != NULL)
    {
        unsigned char* buf = file_buf;
        mp3dec_frame_info_t info;
        mp3dec_t dec;

        mp3dec_init(&dec);
        for (;;)
        {
            int16_t frame_buf[2 * 1152];
            int samples = mp3dec_decode_frame(&dec, buf, music_size, frame_buf, &info);
            if (alloc_samples < (num_samples + samples))
            {
                alloc_samples *= 2;
                int16_t* tmp = (int16_t*)realloc(music_buf, (int)(alloc_samples * 2 * info.channels));
                if (tmp)
                    music_buf = tmp;
            }
            if (music_buf)
                memcpy(music_buf + (int)(num_samples * info.channels), frame_buf, (int)(samples * info.channels * 2));
            num_samples += samples;
            if (info.frame_bytes <= 0 || music_size <= (info.frame_bytes + 4))
                break;
            buf += info.frame_bytes;
            music_size -= info.frame_bytes;
        }
        if (alloc_samples > num_samples)
        {
            int16_t* tmp = (int16_t*)realloc(music_buf, (int)(num_samples * 2 * info.channels));
            if (tmp)
                music_buf = tmp;
        }

        // set return value file size
        if (a_sizeInBytes)
        {
            *a_sizeInBytes = (int)(num_samples * 2 * info.channels);
        }

        // set return value bits per sample
        if (a_bitsPerSample)
            *a_bitsPerSample = 16;

        // set return value bits per sample
        if (a_numSamples)
            *a_numSamples = num_samples;

        // set return value frequency
        if (a_samplingRate)
            *a_samplingRate = info.hz;

        // set return value stereo/mono
        if ((info.channels == 1) && a_stereo)
        {
            *a_stereo = false;
        }
        else if ((info.channels == 2) && a_stereo)
        {
            *a_stereo = true;
        }
        else
        {
            if (file_buf != nullptr)
            {
                free(file_buf);
            }
            return C_ERROR;
        }

        // set return value audio data
        a_data = (unsigned char*)(music_buf);

        // free file
        free(file_buf);
    }

    // return success
    return (C_SUCCESS);
}


//------------------------------------------------------------------------------
} // namespace chai3d
//------------------------------------------------------------------------------
