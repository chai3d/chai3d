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
#ifndef CMultiMeshH
#define CMultiMeshH
//------------------------------------------------------------------------------
#include "world/CMesh.h"
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
namespace chai3d {
//------------------------------------------------------------------------------

//==============================================================================
/*!
    \file       CMultiMesh.h

    \brief 
    Implements a 3D multi-mesh object.
*/
//==============================================================================

//==============================================================================
/*!    
    \class      cMultiMesh
    \ingroup    world

    \brief
    This class implements a 3D multi-mesh object.

    \details
    This class implements a collection of cMesh objects. Each cMesh object
    includes one material and texture properties with a set of vertices
    and triangles. cMultiMesh allows the user to build more complicated 
    polygonal objects composed of sets of triangles that share digfferent
    materials.
*/
//==============================================================================
class cMultiMesh : public cGenericObject
{
    //--------------------------------------------------------------------------
    // CONSTRUCTOR & DESTRUCTOR:
    //--------------------------------------------------------------------------

public:

    //! Constructor of cMultiMesh.
    cMultiMesh();

    //! Destructor of cMultiMesh.
    virtual ~cMultiMesh();


    //--------------------------------------------------------------------------
    // PUBLIC METHODS - GENERAL:
    //--------------------------------------------------------------------------

public:

    //! This method returns the name of the object class.
    virtual std::string getClassName() { return ("MultiMesh"); }

    //! This method creates a copy of itself.
    virtual cMultiMesh* copy(const bool a_duplicateMaterialData = false,
                             const bool a_duplicateTextureData = false, 
                             const bool a_duplicateMeshData = false,
                             const bool a_buildCollisionDetector = true);


    //-----------------------------------------------------------------------
    // PUBLIC METHODS - COLLISION DETECTION:
    //-----------------------------------------------------------------------

public:

    //! Set up a brute force collision detector for this mesh and (optionally) for its children.
    virtual void createBruteForceCollisionDetector();

    //! Set up an AABB collision detector for this mesh.
    virtual void createAABBCollisionDetector(const double a_radius);


    //--------------------------------------------------------------------------
    // PUBLIC METHODS - MESH PRIMITIVES:
    //--------------------------------------------------------------------------

public:

    //! This method creates a new mesh primitive.
    cMesh* newMesh();

    //! This method adds an existing mesh primitives to list of meshes
    bool addMesh(cMesh* a_mesh);

    //! This method removes a mesh primitive from the list of meshes.
    bool removeMesh(cMesh* a_mesh);

    //! This method removes all mesh primitives.
    bool removeAllMesh();

    //! This method deletes a mesh primitive from the list of meshes.
    bool deleteMesh(cMesh* a_mesh);

    //! This method deletes all meshes.
    bool deleteAllMeshes();

    //! This method retrieves the number of meshes that compose this multi-mesh object.
    int getNumMeshes();

    //! This method returns a pointer to a mesh primitive by passing its index number.
    cMesh* getMesh(unsigned int a_index);

    //! This method converts this multimesh into a single mesh object.
    void convertToSingleMesh(cMesh* a_mesh);


    //--------------------------------------------------------------------------
    // PUBLIC METHODS - VERTICES
    //--------------------------------------------------------------------------
    
public:

    //! This method returns the index number and mesh of a specific vertex that is part of this multi-mesh.
    bool getVertex(const unsigned int a_index, cMesh*& a_mesh, unsigned int& a_vertexIndex);

    //! This method returns the position data of specific vertex.
    cVector3d getVertexPos(unsigned int a_index);

    //! This method returns the the number of stored vertices.
    unsigned int getNumVertices() const;

    //! This method enables or disables the use of per-vertex colors, optionally propagating the operation to its children.
    virtual void setUseVertexColors(const bool a_useColors, const bool a_affectChildren=true, const bool a_affectComponents=true);

    //! This method sets the color of each vertex.
    void setVertexColor(const cColorf& a_color);


    //--------------------------------------------------------------------------
    // PUBLIC METHODS - TRIANGLES
    //--------------------------------------------------------------------------

public:

    //! This method returns the index number and mesh of a specific triangle that is part of this multi-mesh.
    bool getTriangle(const unsigned int a_index, cMesh*& a_mesh, unsigned int& a_triangleIndex);

    //! This method returns the the number of stored triangles.
    unsigned int getNumTriangles() const;

    //! This method enables or disables the rendering of triangles.
    void setShowTriangles(const bool a_showTriangles);


    //--------------------------------------------------------------------------
    // PUBLIC METHODS - EDGES
    //--------------------------------------------------------------------------

public:

    //! This method creates a list of edges by providing a threshold angle in degrees.
    void computeAllEdges(double a_angleThresholdDeg = 40.0);

    //! This method clears all edges.
    void clearAllEdges();

    //! This method enables or disables the rendering of edges.
    void setShowEdges(const bool a_showEdges);

    //! This method sets the graphic properties for edge-rendering.
    void setEdgeProperties(const double a_width, 
                           const cColorf& a_color);

    //! This method sets the line width of all edges.
    void setEdgeLineWidth(const double a_width);


    //--------------------------------------------------------------------------
    // PUBLIC METHODS - SURFACE NORMALS
    //--------------------------------------------------------------------------

public:

    //! This method enables or disables the rendering of vertex normals.
    void setShowNormals(const bool& a_showNormals);

    //! This method sets the graphic properties for normal-rendering.
    void setNormalsProperties(const double a_length, const cColorf& a_color);

    //! This method set the length of normals for display purposes.
    void setNormalsLength(const double a_length);

    //! This method computes all triangle normals.
    void computeAllNormals();

    //! This method reverses all normals on this model.
    void reverseAllNormals();

    //! This method computes the normal matrix vectors for all triangles.
    void computeBTN();

    //! This method enables or disables the rendering of tangents and bi-tangents.
    void setShowTangents(const bool a_showTangents);


    //--------------------------------------------------------------------------
    // PUBLIC VIRTUAL METHODS - FILES:
    //--------------------------------------------------------------------------

public:

    //! This method loads a 3D object from a file.
    virtual bool loadFromFile(std::string a_filename);

    //! This method saves 3D object to a file.
    virtual bool saveToFile(std::string a_filename);


    //--------------------------------------------------------------------------
    // PUBLIC METHODS - SCALING:
    //--------------------------------------------------------------------------

public:

    //! This method scales this object by using different factors along X,Y and Z axes.
    void scaleXYZ(const double a_scaleX, const double a_scaleY, const double a_scaleZ);


    //--------------------------------------------------------------------------
    // PROTECTED METHODS:
    //--------------------------------------------------------------------------

protected:

    //! This method copies all properties of this multi-mesh object to another.
    void copyMultiMeshProperties(cMultiMesh* a_obj,
        const bool a_duplicateMaterialData,
        const bool a_duplicateTextureData, 
        const bool a_duplicateMeshData,
        const bool a_buildCollisionDetector);


    //-----------------------------------------------------------------------
    // PUBLIC MEMBERS
    //-----------------------------------------------------------------------

public:

    //! Array of meshes.
    std::vector<cMesh*> *m_meshes;
};

//------------------------------------------------------------------------------
} // namespace chai3d
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
#endif
//------------------------------------------------------------------------------

