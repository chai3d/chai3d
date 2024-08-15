//------------------------------------------------------------------------------

attribute vec3 aPosition;
attribute vec3 aNormal;
attribute vec3 aTexCoord;
attribute vec4 aColor;
attribute vec3 aTangent;
attribute vec3 aBitangent;

// vertex position in model space
varying vec4 vPosition;

//------------------------------------------------------------------------------

void main(void)
{     
    gl_TexCoord[0] = gl_TextureMatrix[0] * vec4(aTexCoord, 1.0);
    vPosition = gl_Vertex;
    gl_Position = ftransform();
}
