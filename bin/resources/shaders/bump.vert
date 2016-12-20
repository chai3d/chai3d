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

//------------------------------------------------------------------------------

varying vec3 vLightVec; 
varying vec3 vEyeVec;
varying vec2 vTexCoord;
varying vec4 vColor;

//------------------------------------------------------------------------------

void main(void)
{
    gl_Position = gl_ModelViewProjectionMatrix * vec4(aPosition, 1);
    vTexCoord = aTexCoord.xy;
    
    vec3 n = normalize(gl_NormalMatrix * aNormal);
    vec3 t = normalize(gl_NormalMatrix * aTangent);
    vec3 b = normalize(gl_NormalMatrix * aBitangent);
    
    vec3 vVertex = vec3(gl_ModelViewMatrix * vec4(aPosition,1));
    vec3 tmpVec = gl_LightSource[0].position.xyz - vVertex;

    vLightVec.x = dot(tmpVec, t);
    vLightVec.y = dot(tmpVec, b);
    vLightVec.z = dot(tmpVec, n);

    tmpVec = -vVertex;
    vEyeVec.x = dot(tmpVec, t);
    vEyeVec.y = dot(tmpVec, b);
    vEyeVec.z = dot(tmpVec, n);

    vColor = aColor;
}

//------------------------------------------------------------------------------