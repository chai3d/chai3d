//==============================================================================
/*
    Software License Agreement (BSD License)
    Copyright (c) 2003-2016, CHAI3D.
    (www.chai3d.org)
*/
//==============================================================================

//------------------------------------------------------------------------------

varying vec3 vLightVec;
varying vec3 vEyeVec;
varying vec2 vTexCoord;
varying vec4 vColor;

//------------------------------------------------------------------------------

uniform sampler2D uColorMap;
uniform sampler2D uNormalMap;
uniform float uInvRadius;

//------------------------------------------------------------------------------

void main (void)
{
    float distSqr = dot(vLightVec, vLightVec);
    float att = clamp(1.0 - uInvRadius * sqrt(distSqr), 0.0, 1.0);
    vec3 lVec = vLightVec * inversesqrt(distSqr);

    vec3 vVec = normalize(vEyeVec);
    
    vec4 base = texture2D(uColorMap, vTexCoord);
    
    vec3 bump = normalize( texture2D(uNormalMap, vTexCoord).xyz * 2.0 - 1.0);

    vec4 vAmbient = gl_LightSource[0].ambient * gl_FrontMaterial.ambient;

    float diffuse = max( dot(lVec, bump), 0.0 );
    
    vec4 vDiffuse = gl_LightSource[0].diffuse * gl_FrontMaterial.diffuse * 
                    diffuse;	

    float specular = pow(clamp(dot(reflect(-lVec, bump), vVec), 0.0, 1.0), 
                     gl_FrontMaterial.shininess );
    
    vec4 vSpecular = gl_LightSource[0].specular * gl_FrontMaterial.specular * 
                     specular;	
    
    gl_FragColor = ( vAmbient*base + vDiffuse*base + vSpecular) * att;
}

//------------------------------------------------------------------------------