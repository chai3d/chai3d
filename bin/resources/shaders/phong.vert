//======================================================================
/*
 Software License Agreement (BSD License)
 Copyright (c) 2003-2016, CHAI3D.
 (www.chai3d.org)
 
 \author    Sonny Chan
 */
//======================================================================
#version 120

attribute vec3 aPosition;
attribute vec3 aNormal;
attribute vec3 aTexCoord;
attribute vec4 aColor;
attribute vec3 aTangent;
attribute vec3 aBitangent;

// vertex position and normal in eye coordinate space
varying vec4 vPosition;
varying vec3 vNormal;

//----------------------------------------------------------------------
// Main vertex shader code.
//----------------------------------------------------------------------

void main(void)
{
    // pass along a transformed vertex position, normal, and texture
    vPosition = gl_ModelViewMatrix * gl_Vertex;
    vNormal = gl_NormalMatrix * aNormal;
    gl_TexCoord[0] = gl_TextureMatrix[0] * vec4(aTexCoord, 1.0);
    
    // transform the vertex to get shadow map texture coordinates
    float s = dot(gl_EyePlaneS[1], vPosition);
    float t = dot(gl_EyePlaneT[1], vPosition);
    float r = dot(gl_EyePlaneR[1], vPosition);
    float q = dot(gl_EyePlaneQ[1], vPosition);
    gl_TexCoord[1] = vec4(s, t, r, q);
    
    // fixed function vertex transform
    gl_Position = ftransform();
}

//======================================================================
