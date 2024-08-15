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
#include "world/CMultiMesh.h"
//------------------------------------------------------------------------------
#include "collisions/CGenericCollision.h"
#include "collisions/CCollisionBrute.h"
#include "collisions/CCollisionAABB.h"
#include "files/CFileModel3DS.h"
#include "files/CFileModelOBJ.h"
#include "files/CFileModelSTL.h"
#include "files/CFileModelTRI.h"
#include "math/CMaths.h"
//------------------------------------------------------------------------------
#include <float.h>
#include <algorithm>
#include <set>
//------------------------------------------------------------------------------
using namespace std;
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
namespace chai3d {
//------------------------------------------------------------------------------

//==============================================================================
/*!
    Constructor of cMultiMesh.
*/
//==============================================================================
cMultiMesh::cMultiMesh()
{
    // create array of mesh primitives
    m_meshes = new vector<cMesh*>;
}


//==============================================================================
/*!
    Destructor of cMultiMesh.
*/
//==============================================================================
cMultiMesh::~cMultiMesh()
{
    // delete all meshes
    deleteAllMeshes();

    // delete mesh vector
    delete m_meshes;
}


//==============================================================================
/*!
    This method creates a copy of itself.

    \param  a_duplicateMaterialData   If __true__, material (if available) is duplicated, otherwise it is shared.
    \param  a_duplicateTextureData    If __true__, texture data (if available) is duplicated, otherwise it is shared.
    \param  a_duplicateMeshData       If __true__, mesh data (if available) is duplicated, otherwise it is shared.
    \param  a_buildCollisionDetector  If __true__, collision detector (if available) is duplicated, otherwise it is shared.

    \return Pointer to new object.
*/
//==============================================================================
cMultiMesh* cMultiMesh::copy(const bool a_duplicateMaterialData,
                             const bool a_duplicateTextureData, 
                             const bool a_duplicateMeshData,
                             const bool a_buildCollisionDetector)
{
    // create multimesh object
    cMultiMesh* obj = new cMultiMesh();

    // copy multimesh properties
    copyMultiMeshProperties(obj,
        a_duplicateMaterialData,
        a_duplicateTextureData, 
        a_duplicateMeshData,
        a_buildCollisionDetector);

    // return result
    return (obj);
}


//==============================================================================
/*!
    This method copies all properties of this multi-mesh to another.

    \param  a_obj                     Destination object where properties are copied to.
    \param  a_duplicateMaterialData   If __true__, material (if available) is duplicated, otherwise it is shared.
    \param  a_duplicateTextureData    If __true__, texture data (if available) is duplicated, otherwise it is shared.
    \param  a_duplicateMeshData       If __true__, mesh data (if available) is duplicated, otherwise it is shared.
    \param  a_buildCollisionDetector  If __true__, collision detector (if available) is duplicated, otherwise it is shared.
*/
//==============================================================================
void cMultiMesh::copyMultiMeshProperties(cMultiMesh* a_obj,
    const bool a_duplicateMaterialData,
    const bool a_duplicateTextureData, 
    const bool a_duplicateMeshData,
    const bool a_buildCollisionDetector)
{
    // copy properties of cGenericObject
    copyGenericObjectProperties(a_obj, 
        a_duplicateMaterialData, 
        a_duplicateTextureData,
        a_duplicateMeshData,
        a_buildCollisionDetector);

    // create properties of cMultiMesh
    vector<cMesh*>::iterator it;
    for (it = m_meshes->begin(); it < m_meshes->end(); it++)
    {
        cMesh* mesh = (*it)->copy(a_duplicateMaterialData,
            a_duplicateTextureData, 
            a_duplicateMeshData,
            a_buildCollisionDetector);

        a_obj->addMesh(mesh);
    }
}


//==============================================================================
/*!
    This method enables or disables the use of per-vertex color information of
    when rendering the mesh.

    \param  a_useColors         If __true__ then then vertex color information is 
                                applied.
    \param  a_affectChildren    If __true__ then children are updated too.
    \param  a_affectComponents  If __true__, then components are updated too.
*/
//==============================================================================
void cMultiMesh::setUseVertexColors(const bool a_useColors, 
                                    const bool a_affectChildren,
                                    const bool a_affectComponents)
{
    // update current object and possibly children
    cGenericObject::setUseVertexColors(a_useColors, a_affectChildren, a_affectComponents);

    // updated meshes
    vector<cMesh*>::iterator it;
    for (it = m_meshes->begin(); it < m_meshes->end(); it++)
    {
        (*it)->setUseVertexColors(a_useColors, true, a_affectComponents);
    }
}


//==============================================================================
/*!
    This method scales this mesh by using different scale factors along X, Y, 
    and Z axes.

    \param  a_scaleX  Scale factor along X axis.
    \param  a_scaleY  Scale factor along Y axis.
    \param  a_scaleZ  Scale factor along Z axis.
*/
//==============================================================================
void cMultiMesh::scaleXYZ(const double a_scaleX, const double a_scaleY, const double a_scaleZ)
{
    vector<cMesh*>::iterator it;
    for (it = m_meshes->begin(); it < m_meshes->end(); it++)
    {
        cMatrix3d a_R_b = (*it)->getLocalRot();
        cMatrix3d b_R_a = cTranspose(a_R_b);

        // scale vertices
        unsigned int numVertices = (*it)->m_vertices->getNumElements();
        for (unsigned int i=0; i<numVertices; i++)
        {
            cVector3d b_Vertex =  (*it)->m_vertices->getLocalPos(i);
            cVector3d a_Vertex = a_R_b *  b_Vertex;
            a_Vertex = a_R_b * b_Vertex;
            a_Vertex.mul(a_scaleX, a_scaleY, a_scaleZ);
            b_Vertex = b_R_a * a_Vertex;
            (*it)->m_vertices->setLocalPos(i, b_Vertex);
        }

        // scale position
        (*it)->m_localPos.mul(a_scaleX, a_scaleY, a_scaleZ);

        // update boundary box
        cVector3d b_BoxMin = (*it)->m_boundaryBoxMin;
        cVector3d a_BoxMin = a_R_b * b_BoxMin;
        a_BoxMin.mul(a_scaleX, a_scaleY, a_scaleZ);
        b_BoxMin = b_R_a * a_BoxMin;
        (*it)->m_boundaryBoxMin = b_BoxMin;

        cVector3d b_BoxMax = (*it)->m_boundaryBoxMax;
        cVector3d a_BoxMax = a_R_b * b_BoxMax;
        a_BoxMax.mul(a_scaleX, a_scaleY, a_scaleZ);
        b_BoxMax = b_R_a * a_BoxMax;
        (*it)->m_boundaryBoxMax = b_BoxMax;
    }

    // update boundary box
    m_boundaryBoxMax.mul(a_scaleX, a_scaleY, a_scaleZ);
    m_boundaryBoxMin.mul(a_scaleX, a_scaleY, a_scaleZ);
}


//==============================================================================
/*!
    This method creates a new mesh and adds it to the list of meshes.

    \return Pointer to new mesh object.
*/
//==============================================================================
cMesh* cMultiMesh::newMesh()
{
    // create new mesh entity
    cMesh* obj = new cMesh();

    // add mesh to component list
    addComponent(obj);

    // add mesh to list
    m_meshes->push_back(obj);

    // return result
    return (obj);
}


//==============================================================================
/*!
    This method adds an existing mesh primitives to list of meshes

    \return __true__ if the operation succeeds, __false__ otherwise.
*/
//==============================================================================
bool cMultiMesh::addMesh(cMesh* a_mesh)
{
    // sanity check
    if (a_mesh == NULL) { return (false); }
    if (a_mesh->getParent() != NULL) { return (false); }

    // add mesh to component list
    addComponent(this);

    // add mesh to list of meshes
    m_meshes->push_back(a_mesh);

    // return success
    return (true);
}


//==============================================================================
/*!
    This method removes a mesh primitive from the list of meshes.

    \param  a_mesh  Mesh to remove.

    \return __true__ if the operation succeeds, __false__ otherwise.
*/
//==============================================================================
bool cMultiMesh::removeMesh(cMesh* a_mesh)
{
    // remove mesh from list of components
    removeComponent(a_mesh);

    // remove mesh from list of meshes
    vector<cMesh*>::iterator it;
    for (it = m_meshes->begin(); it < m_meshes->end(); it++)
    {
        if ((*it) == a_mesh)
        {
            // remove mesh from list of meshes
            m_meshes->erase(it);

            // return success
            return (true);
        }
    }
    return (false);
}


//==============================================================================
/*!
    This method removes all mesh primitives.

    \return __true__ if the operation succeeds, __false__ otherwise.
*/
//==============================================================================
bool cMultiMesh::removeAllMesh()
{
    vector<cMesh*>::iterator it;
    for (it = m_meshes->begin(); it < m_meshes->end(); it++)
    {
        // remove mesh from component list
        removeComponent(*it);

        // parent is set to NULL. Mesh becomes its own owner.
        (*it)->setParent(NULL);
        (*it)->setOwner(*it);
    }

    // clear list of meshes
    m_meshes->clear();

    // success
    return (true);
}


//==============================================================================
/*!
    This method deletes a mesh primitive from the list of meshes.

    \param  a_mesh  Mesh to delete.

    \return __true__ if the operation succeeds, __false__ otherwise.
*/
//==============================================================================
bool cMultiMesh::deleteMesh(cMesh* a_mesh)
{
    vector<cMesh*>::iterator it;
    for (it = m_meshes->begin(); it < m_meshes->end(); it++)
    {
        if ((*it) == a_mesh)
        {
            // remove mesh from component list
            removeComponent(*it);

            // remove mesh from list of meshes
            m_meshes->erase(it);

            // delete mesh
            delete (*it);

            // return success
            return (true);
        }
    }

    // operation failed, mesh was not found
    return (false);
}


//==============================================================================
/*!
    This method deletes all meshes.

    \return __true__ if the operation succeeds, __false__ otherwise.
*/
//==============================================================================
bool cMultiMesh::deleteAllMeshes()
{
    // delete all meshes
    vector<cMesh*>::iterator it;
    for (it = m_meshes->begin(); it < m_meshes->end(); it++)
    {
        // get next mesh
        cMesh* nextMesh = (*it);
        
        // remove mesh from component list
        removeComponent(nextMesh);

        // delete mesh from memory
        delete nextMesh;
    }

    // clear all meshes from list
    m_meshes->clear();

    // success
    return (true);
}


//==============================================================================
/*!
    This method retrieves the number of meshes that compose this multi-mesh object.

    \return Number of meshes contained in this object.
*/
//==============================================================================
int cMultiMesh::getNumMeshes()
{
    unsigned int numMeshes = (unsigned int)(m_meshes->size());
    return (numMeshes);
}


//==============================================================================
/*!
    This method returns a pointer to a mesh primitive by passing its index number.

    \param  a_index  Index number of mesh.

    \return Pointer to selected mesh.
*/
//==============================================================================
cMesh* cMultiMesh::getMesh(unsigned  int a_index)
{
    if (a_index < m_meshes->size())
    {
        return (m_meshes->at(a_index));
    }
    else
    {
        return (NULL);
    }
}


//==============================================================================
/*!
    This method returns the index number and mesh of a specific triangle that
    is part of this multi-mesh.

    \param  a_index          Index number of the requested triangle.
    \param  a_mesh           Pointer to the mesh containing the selected triangle.
    \param  a_triangleIndex  Index number of the specified triangle inside the mesh triangle array.
*/
//==============================================================================
bool cMultiMesh::getTriangle(const unsigned int a_index, cMesh*& a_mesh, unsigned int& a_triangleIndex)
{
    unsigned int index = a_index;
    unsigned int i, numMeshes;
    numMeshes = (unsigned int)(m_meshes->size());
    for (i=0; i<numMeshes; i++)
    {
        cMesh* nextMesh = m_meshes->at(i);
        if (nextMesh)
        {
             unsigned int numTriangles = nextMesh->getNumTriangles();

             if (index < numTriangles)
             {
                a_triangleIndex = index;
                a_mesh = nextMesh;
                return (true);
             }
             else 
             {
                index -= numTriangles;
             }
        }
    }

    a_mesh = NULL;
    a_triangleIndex = 0;
    return (false);
}


//==============================================================================
/*!
    This method returns the index number and mesh of a specific vertex that is 
    part of this multi-mesh.

    \param  a_index        Index number of the requested vertex.
    \param  a_mesh         Pointer to the mesh containing the selected vertex.
    \param  a_vertexIndex  Index number of the specified vertex inside the mesh vertex array.
*/
//==============================================================================
bool cMultiMesh::getVertex(const unsigned int a_index, cMesh*& a_mesh, unsigned int& a_vertexIndex)
{
    unsigned int index = a_index;
    unsigned int i, numMeshes;
    numMeshes = (unsigned int)(m_meshes->size());
    for (i=0; i<numMeshes; i++)
    {
        cMesh* nextMesh = m_meshes->at(i);
        if (nextMesh)
        {
             unsigned int numVertices = nextMesh->getNumVertices();

             if (index < numVertices)
             {
                a_vertexIndex = index;
                a_mesh = nextMesh;
                return (true);
             }
             else 
             {
                index -= numVertices;
             }
        }
    }

    a_mesh = NULL;
    a_vertexIndex = 0;
    return (false);
}


//==============================================================================
/*!
    This method returns the the number of stored triangles.

    \return Number of triangles.
*/
//==============================================================================
unsigned int cMultiMesh::getNumTriangles() const
{
    int numTriangles = 0;

    // count triangles of all meshes that compose this multi-mesh
    vector<cMesh*>::iterator it;
    for (it = m_meshes->begin(); it < m_meshes->end(); it++)
    {
        numTriangles = numTriangles + (*it)->getNumTriangles();
    }

    return (numTriangles);
}


//==============================================================================
/*!
    This method returns the position data of specific vertex.

    \param  a_index  Index of the requested vertex.

    \return Local position value at vertex.
*/
//==============================================================================
cVector3d cMultiMesh::getVertexPos(unsigned int a_index)
    
{
    // sanity check
    if (a_index >= getNumVertices()) return (NULL);

    // retrieve triangle
    unsigned int i, numMeshes;
    numMeshes = (unsigned int)(m_meshes->size());
    for (i=0; i<numMeshes; i++)
    {
        cMesh* nextMesh = m_meshes->at(i);
        if (nextMesh)
        {
             unsigned int numVertices = nextMesh->getNumVertices();

             if (a_index < numVertices)
             {
                return (nextMesh->m_vertices->getLocalPos(a_index));
             }
             else 
             {
                a_index -= numVertices;
             }
        }
    }

    return (cVector3d(0,0,0));
}


//==============================================================================
/*!
    This method returns the the number of stored vertices.

    \return Number of vertices.
*/
//==============================================================================
unsigned int cMultiMesh::getNumVertices() const
{
    int numVertices = 0;

    vector<cMesh*>::iterator it;
    for (it = m_meshes->begin(); it < m_meshes->end(); it++)
    {
        numVertices = numVertices + (*it)->getNumVertices();
    }

    return (numVertices);
}


//==============================================================================
/*!
    This method converts this multimesh into a single mesh object. 
    Material and texture properties are not copied.
*/
//==============================================================================
void cMultiMesh::convertToSingleMesh(cMesh* a_mesh)
{
    // clear all previous data
    a_mesh->clear();

    // store vertices for each mesh
    vector<cMesh*>::iterator it;
    for (it = m_meshes->begin(); it < m_meshes->end(); it++)
    {
        cMesh* nextMesh = (*it);
        cVector3d posMesh = nextMesh->getLocalPos();
        cMatrix3d rotMesh = nextMesh->getLocalRot();

        int numVertices = nextMesh->getNumVertices();

        for (int i=0; i<numVertices; i++)
        {
            cVector3d pos = posMesh + rotMesh * nextMesh->m_vertices->getLocalPos(i);
            cVector3d normal = rotMesh * nextMesh->m_vertices->getNormal(i);
            cVector3d texCoord = nextMesh->m_vertices->getTexCoord(i);
            cColorf   color = nextMesh->m_vertices->getColor(i);

            a_mesh->newVertex(pos, normal, texCoord, color);
        }
    }

    // store triangles for each mesh
    int vertexOffset = 0;
    for (it = m_meshes->begin(); it < m_meshes->end(); it++)
    {
        cMesh* nextMesh = (*it);

        int numTriangles = nextMesh->getNumTriangles();

        for (int i=0; i<numTriangles; i++)
        {
            unsigned int vertex0 = nextMesh->m_triangles->getVertexIndex0(i) + vertexOffset;
            unsigned int vertex1 = nextMesh->m_triangles->getVertexIndex1(i) + vertexOffset;
            unsigned int vertex2 = nextMesh->m_triangles->getVertexIndex2(i) + vertexOffset;

            a_mesh->newTriangle(vertex0, vertex1, vertex2);
        }

        vertexOffset = vertexOffset + nextMesh->getNumVertices();
    }
}



//==============================================================================
/*!
    This method loads a 3D mesh file. \n
    CHAI3D currently supports .obj, .3ds, and .stl files.

    \param  a_filename  Filename of 3D model.

    \return __true__ if the operation succeeds, __false__ otherwise.
*/
//==============================================================================
bool cMultiMesh::loadFromFile(string a_filename)
{ 
    // find extension
    string extension = cGetFileExtension(a_filename);

    // we need a file extension to figure out file type
    if (extension.length() == 0)
    {
        return (false);
    }

    // convert string to lower extension
    string fileType = cStrToLower(extension);

    // result for loading file
    bool result = false;

    //--------------------------------------------------------------------
    // .OBJ FORMAT
    //--------------------------------------------------------------------
    if (fileType == "obj")
    {
        result = cLoadFileOBJ(this, a_filename);
    }

#ifndef CHAI3D_LIB3DS_DISABLED

    //--------------------------------------------------------------------
    // .3DS FORMAT
    //--------------------------------------------------------------------
    else if (fileType == "3ds")
    {
        result = cLoadFile3DS(this, a_filename);
    }

#endif

    //--------------------------------------------------------------------
    // .STL FORMAT
    //--------------------------------------------------------------------
    else if (fileType == "stl")
    {
        result = cLoadFileSTL(this, a_filename);
    }

    return (result);
}


//==============================================================================
/*!
    This method saves a mesh object to file. \n
    CHAI3D currently supports .obj, .3ds, and .stl files.

    \param  a_filename  Filename of 3D model.

    \return __true__ if the operation succeeds, __false__ otherwise.
*/
//==============================================================================
bool cMultiMesh::saveToFile(std::string a_filename)
{ 
    // find extension
    string extension = cGetFileExtension(a_filename);

    // we need a file extension to figure out file type
    if (extension.length() == 0)
    {
        return (false);
    }

    // convert string to lower extension
    string fileType = cStrToLower(extension);

    // result for loading file
    bool result = false;

    //--------------------------------------------------------------------
    // .OBJ FORMAT
    //--------------------------------------------------------------------
    if (fileType == "obj")
    {
        result = cSaveFileOBJ(this, a_filename);
    }

#ifndef CHAI3D_LIB3DS_DISABLED

    //--------------------------------------------------------------------
    // .3DS FORMAT
    //--------------------------------------------------------------------
    else if (fileType == "3ds")
    {
        result = cSaveFile3DS(this, a_filename);
    }

#endif

    //--------------------------------------------------------------------
    // .STL FORMAT
    //--------------------------------------------------------------------
    else if (fileType == "stl")
    {
        result = cSaveFileSTL(this, a_filename);
    }

    //--------------------------------------------------------------------
    // .TRI FORMAT
    //--------------------------------------------------------------------
    else if (fileType == "tri")
    {
        result = cSaveFileTRI(this, a_filename);
    }

    return (result);
}


//==============================================================================
/*!
    This method computes all triangle normals.
*/
//==============================================================================
void cMultiMesh::computeAllNormals()
{
    vector<cMesh*>::iterator it;
    for (it = m_meshes->begin(); it < m_meshes->end(); it++)
    {
        (*it)->computeAllNormals();
    }
}


//==============================================================================
/*!
    This method computes the normal matrix vectors for all triangles.
*/
//==============================================================================
void cMultiMesh::computeBTN()
{
    vector<cMesh*>::iterator it;
    for (it = m_meshes->begin(); it < m_meshes->end(); it++)
    {
        (*it)->computeBTN();
    }
}



//==============================================================================
/*!
    This method enables or disables the rendering of tangents and bi-tangents.

    \param  a_showTangents  Display mode.
*/
//==============================================================================
void cMultiMesh::setShowTangents(const bool a_showTangents)
{
    vector<cMesh*>::iterator it;
    for (it = m_meshes->begin(); it < m_meshes->end(); it++)
    {
        (*it)->setShowTangents(a_showTangents);
    }
}


//==============================================================================
/*!
    This method enables or disables the rendering of triangles.

    \param  a_showTriangles  If __true__ then triangles are rendered, __false__ otherwise.
*/
//==============================================================================
void cMultiMesh::setShowTriangles(const bool a_showTriangles)
{
    vector<cMesh*>::iterator it;
    for (it = m_meshes->begin(); it < m_meshes->end(); it++)
    {
        (*it)->setShowTriangles(a_showTriangles);
    }
}


//==============================================================================
/*!
    This method enables or disables the rendering of edges.

    \param  a_showEdges  If __true__ then edges are rendered, __false__ otherwise.
*/
//==============================================================================
void cMultiMesh::setShowEdges(const bool a_showEdges)
{
    vector<cMesh*>::iterator it;
    for (it = m_meshes->begin(); it < m_meshes->end(); it++)
    {
        (*it)->setShowEdges(a_showEdges);
    }
}


//==============================================================================
/*!
    This method sets the graphic properties for edge-rendering. Options passed
    by argument include the width of the edges and their color.

    \param  a_width  Width of edge lines.
    \param  a_color  Color of edge lines.
*/
//==============================================================================
void cMultiMesh::setEdgeProperties(const double a_width, 
                                   const cColorf& a_color)
{
    vector<cMesh*>::iterator it;
    for (it = m_meshes->begin(); it < m_meshes->end(); it++)
    {
        (*it)->setEdgeProperties(a_width, a_color);
    }
}


//==============================================================================
/*!
    This method sets the line width of all edges.

    \param  a_width  Width of edge lines.
*/
//==============================================================================
void cMultiMesh::setEdgeLineWidth(const double a_width)
{
    vector<cMesh*>::iterator it;
    for (it = m_meshes->begin(); it < m_meshes->end(); it++)
    {
        (*it)->setEdgeLineWidth(a_width);
    }
}


//==============================================================================
/*!
    This method creates a list of edges by providing a threshold angle in 
    degrees. All triangles for which the angle between their respective surface 
    normals are greater than the select angle threshold are added to the list of 
    edges.

    \param  a_angleThresholdDeg  Threshold angle in degrees.
*/
//==============================================================================
void cMultiMesh::computeAllEdges(double a_angleThresholdDeg)
{
    vector<cMesh*>::iterator it;
    for (it = m_meshes->begin(); it < m_meshes->end(); it++)
    {
        (*it)->computeAllEdges(a_angleThresholdDeg);
    }
}


//==============================================================================
/*!
    This method clears all edges.
*/
//==============================================================================
void cMultiMesh::clearAllEdges()
{
    vector<cMesh*>::iterator it;
    for (it = m_meshes->begin(); it < m_meshes->end(); it++)
    {
        (*it)->clearAllEdges();
    }
}


//==============================================================================
/*!
    This method sets the color of each vertex.

    \param  a_color  New color to be applied to each vertex.
*/
//==============================================================================
void cMultiMesh::setVertexColor(const cColorf& a_color)
{
    vector<cMesh*>::iterator it;
    for (it = m_meshes->begin(); it < m_meshes->end(); it++)
    {
        (*it)->setVertexColor(a_color);
    }
}


//==============================================================================
/*!
    This method reverses the normal for every vertex on this model. Useful 
    for models that started with inverted faces and thus gave inward-pointing
    normals.
*/
//==============================================================================
void cMultiMesh::reverseAllNormals()
{
    vector<cMesh*>::iterator it;
    for (it = m_meshes->begin(); it < m_meshes->end(); it++)
    {
        (*it)->reverseAllNormals();
    }
}


//==============================================================================
/*!
    This method defines the way normals are graphically rendered.

    \param  a_length  Length of normals.
    \param  a_color   Color of normals.
*/
//==============================================================================
void cMultiMesh::setNormalsProperties(const double a_length, 
                                      const cColorf& a_color)
{
    vector<cMesh*>::iterator it;
    for (it = m_meshes->begin(); it < m_meshes->end(); it++)
    {
        (*it)->setNormalsProperties(a_length, a_color);
    }
}


//==============================================================================
/*!
    This method set the length of normals for display purposes.

    \param  a_length  Length of normals.
*/
//==============================================================================
void cMultiMesh::setNormalsLength(const double a_length)
{
    vector<cMesh*>::iterator it;
    for (it = m_meshes->begin(); it < m_meshes->end(); it++)
    {
        (*it)->setNormalsLength(a_length);
    }
}


//==============================================================================
/*!
    This method enables or disables the rendering of vertex normals.

    \param  a_showNormals  If __true__ then normal vectors are rendered graphically, __false__ otherwise.
*/
//==============================================================================
void cMultiMesh::setShowNormals(const bool& a_showNormals)
{
    vector<cMesh*>::iterator it;
    for (it = m_meshes->begin(); it < m_meshes->end(); it++)
    {
        (*it)->setShowNormals(a_showNormals);
    }
}


//==============================================================================
/*!
     This method builds a brute Force collision detector for this mesh.
*/
//==============================================================================
void cMultiMesh::createBruteForceCollisionDetector()
{
    vector<cMesh*>::iterator it;
    for (it = m_meshes->begin(); it < m_meshes->end(); it++)
    {
        (*it)->createBruteForceCollisionDetector();
    }
}


//==============================================================================
/*!
    This method builds an AABB collision detector for this mesh.

    \param  a_radius  Bounding radius.
*/
//==============================================================================
void cMultiMesh::createAABBCollisionDetector(const double a_radius)
{
    vector<cMesh*>::iterator it;
    for (it = m_meshes->begin(); it < m_meshes->end(); it++)
    {
        (*it)->createAABBCollisionDetector(a_radius);
    }
}


//------------------------------------------------------------------------------
} // namespace chai3d
//------------------------------------------------------------------------------
