//===========================================================================
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
//===========================================================================

//---------------------------------------------------------------------------
#include <iostream>
#include <iomanip>
#include <ostream>
#include <fstream>
#include <algorithm>
#include <string>
#include <cstring>
using namespace std;
//---------------------------------------------------------------------------
#include "chai3d.h"
using namespace chai3d;
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// DECLARED FUNCTIONS
//---------------------------------------------------------------------------

// simple usage printer
int usage()
{
    cout << endl << "obj2tri file.obj" << endl;
    cout << "\t-h\tdisplay this message" << endl << endl;

    return -1;
}


//===========================================================================
/*
    UTILITY:    cfont.cpp

    This utility takes font information from a .fnt file. It reads the
    corresponding image (that can be in any CHAI3D supported format) and
    produces a C/C++ compatible header containing the font information.
    This allows programmers to easily embed fonts into their executables.
 */
//===========================================================================

int main(int argc, char* argv[])
{
    std::cout << "Have " << argc << " arguments:" << std::endl;
    if (argc < 2)
    {
        usage();
        return -1;
    }

    // get filename
    std::string filename = argv[1];

    // pretty message
    cout << endl;
    cout << "-----------------------------------" << endl;
    cout << "CHAI3D" << endl;
    cout << "obj2tri file converter" << endl;
    cout << "Copyright 2003-2021" << endl;
    cout << "-----------------------------------" << endl;
    cout << endl;

    cout << "loading file: " << filename << endl;

    g_objLoaderUnifyVerticesWithSamePosition = false;
    g_objLoaderShouldGenerateExtraVertices = false;
    g_objLoaderMinimizeNumberOfMeshes = false;

    cMultiMesh* multiMesh = new cMultiMesh();
    bool success = multiMesh->loadFromFile(filename);

    if (!success)
    {
        cout << "failed to open file." << endl;
        return -1;
    }

    int num = multiMesh->getNumMeshes();

    cout << "converting " << num << "meshes." << endl;

    for (int i = 0; i < num; i++)
    {
        // create temporary multimesh
        cMultiMesh* part = new cMultiMesh();

        // copy ith mesh of input model
        cMesh* mesh = multiMesh->getMesh(i)->copy();
        part->addMesh(mesh);

        // create new file name
        std::string newFileName = filename + "_" + cStr(i) + "_" + multiMesh->getMesh(i)->m_name + ".tri";

        // display message
        cout << "saving mesh: " << newFileName << endl;

        // save .tri file
        part->saveToFile(newFileName);

        // save .3ds file
        //newFileName = filename + "_" + cStr(i) + ".3ds";
        //part->saveToFile(newFileName);

        delete part;
    }

    return 0;
}

//---------------------------------------------------------------------------
