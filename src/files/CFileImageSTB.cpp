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
#include "files/CFileImageSTB.h"
//------------------------------------------------------------------------------
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
using namespace chai3d;
namespace chai3d {
//------------------------------------------------------------------------------

//==============================================================================
/*!
    This function blah blah blah.

    \param  a_image     Image structure.
    \param  a_filename  Filename.

    \return __true__ in case of success, __false__ otherwise.
*/
//==============================================================================
bool cLoadFileSTB(cImage* a_image, const std::string& a_filename)
{
    int width, height, numChannels;
    unsigned int bytesPerChannel = 0;
    unsigned int type = 0;
    void *data = NULL;
    unsigned char *data8 = NULL;
    unsigned short *data16 = NULL;

    // request inversion required by textures
    stbi_set_flip_vertically_on_load(true);

    // try 8-bit per channel load
    data8 = stbi_load(a_filename.c_str(), &width, &height, &numChannels, 0);
    if (data8)
    {
        data = data8;
        bytesPerChannel = 1;
        type = GL_UNSIGNED_BYTE;
    }

    // if failed, try 16-bit per channel load
    else data16 = stbi_load_16(a_filename.c_str(), &width, &height, &numChannels, 0);
    if (data16)
    {
        data = data16;
        bytesPerChannel = 2;
        type = GL_UNSIGNED_SHORT;
    }

    // otherwise, failure
    if (!data)
    {
        return (C_ERROR);
    }
    
    // determine format
    GLenum format;
    switch(numChannels)
    {
    case 1:
        format = GL_LUMINANCE;
        break;
    case 3:
        format = GL_RGB;
        break;
    case 4:
        format = GL_RGBA;
        break;
    default:
        stbi_image_free(data);
        return (C_ERROR);
    }

    // allocate image
    a_image->setData((unsigned char*)data, bytesPerChannel*width*height*numChannels, true);
    a_image->setProperties((unsigned int)width, (unsigned int)height, format, type);

    // return success
    return (C_SUCCESS);
}


//------------------------------------------------------------------------------
} // namespace chai3d
//------------------------------------------------------------------------------
