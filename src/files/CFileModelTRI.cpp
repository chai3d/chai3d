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
#include "files/CFileModelTRI.h"
//------------------------------------------------------------------------------
#include "stdint.h"
#include <stdio.h>
#include <math.h>
#include <fstream>
//------------------------------------------------------------------------------
using namespace std;
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
using namespace chai3d;
namespace chai3d {
//------------------------------------------------------------------------------

//==============================================================================
/*!
    This function saves an TRI format 3D model from a cMultiMesh object to a file.
    If the operation succeeds, then the functions returns __true__ and the 
    model data is saved to a file.
    If the operation fails, then the function returns __false__.

    \param  a_object    Multimesh object.
    \param  a_filename  Filename.

    \return __true__ if in case of success, __false__ otherwise.
*/
//==============================================================================
bool cSaveFileTRI(cMultiMesh* a_object, const std::string& a_filename)
{
    // sanity check
    if (a_object == NULL)
    {
        return (C_ERROR);
    }

    // get number of meshes
    unsigned int numMeshes = a_object->getNumMeshes();

    string filePath = cGetDirectory(a_filename);
    string fileName = cGetFilename(a_filename, false);

    // save file for each mesh
    for (unsigned int i = 0; i < numMeshes; i++)
    {
        // set filename
        string fileStr;
        if (numMeshes > 1)
        {
            fileStr = filePath + fileName + "_" + cStr(i) + ".tri";
        }
        else
        {
            fileStr = a_filename;
        }

        // create file
        ofstream file(fileStr.c_str());
        if (!file)
        {
            return (C_ERROR);
        }

        // get mesh
        cMesh* mesh = a_object->getMesh(i);

        // get number of vertices
        unsigned int numVertices = mesh->getNumVertices();

        // get number of triangles
        unsigned int numTriangles = mesh->getNumTriangles();

        // write header data
        file << "TRI" << endl;
        file << numVertices << endl;
        file << numTriangles << endl;

        // write all vertices
        for (unsigned int j = 0; j<numVertices; j++)
        {
            // get position
            cVector3d p = mesh->m_vertices->getLocalPos(j);

            // write data
            file << cStr(p(0), 8) << " " << cStr(p(1), 8) << " " << cStr(p(2), 8) << " " << endl;
        }

        // write all triangles
        for (unsigned int j = 0; j<numTriangles; j++)
        {
            int index0 = mesh->m_triangles->getVertexIndex0(j);
            int index1 = mesh->m_triangles->getVertexIndex1(j);
            int index2 = mesh->m_triangles->getVertexIndex2(j);
            file << index0 << " " << index1 << " " << index2 << " " << endl;
        }

        // close file
        file.close();
    }

    // return success
    return (C_SUCCESS);
}


//------------------------------------------------------------------------------
} // namespace chai3d
//------------------------------------------------------------------------------
