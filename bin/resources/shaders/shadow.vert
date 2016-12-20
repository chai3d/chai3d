//==============================================================================
/*
    Software License Agreement (BSD License)
    Copyright (c) 2003-2016, CHAI3D.
    (www.chai3d.org)
*/
//==============================================================================

//------------------------------------------------------------------------------

attribute  vec3 aPosition;
attribute  vec3 aNormal;
attribute  vec3 aTexCoord;
attribute  vec4 aColor;
attribute  vec3 aTangent;
attribute  vec3 aBitangent;

// Used for shadow lookup
varying vec4 ShadowCoord;
varying vec4 color;
	
	void main()
	{
		ShadowCoord= gl_TextureMatrix[1] * gl_Vertex;
	  
    gl_Position = gl_ModelViewProjectionMatrix * vec4(aPosition, 1);
	
		gl_FrontColor = aColor;
		color = aColor;
	}
