//======================================================================
/*
    Software License Agreement (BSD License)
    Copyright (c) 2003-2016, CHAI3D.
    (www.chai3d.org)
 
    \author    Sonny Chan
 */
//======================================================================

// interpolated vertex position in model space (from vertex shader)
varying vec4 vPosition;

uniform vec3 uMinCorner;        // lower bounding corner, model space
uniform vec3 uMaxCorner;        // upper bounding corner, model space
uniform vec3 uTextureScale;     // convert model to texture coordinates
uniform vec3 uGradientDelta;    // one voxel step in texture space

uniform sampler3D uVolume;      // volume texture
uniform sampler1D uColorLUT;    // 1D transfer function texture

uniform float uIsosurface;      // isosurface level
uniform float uResolution;      // max samples to take on ray traversal

// deltas for gradient estimation along each axis in texture space
vec3 dx = vec3(uGradientDelta.x, 0.0, 0.0);
vec3 dy = vec3(0.0, uGradientDelta.y, 0.0);
vec3 dz = vec3(0.0, 0.0, uGradientDelta.z);

//----------------------------------------------------------------------
// Finds the exiting intersection between a ray e0+d and the volume's
// bounding box
//----------------------------------------------------------------------

float exit(vec3 e0, vec3 d)
{
    float t = distance(uMinCorner, uMaxCorner);
    
    vec3 a = (uMinCorner - e0) / d;
    vec3 b = (uMaxCorner - e0) / d;
    vec3 u = max(a, b);
    
    return min( min(t, u.x), min(u.y, u.z) );
}

float entry(vec3 e1, vec3 d)
{
    float t = distance(uMinCorner, uMaxCorner);
    
    vec3 a = (uMinCorner - e1) / d;
    vec3 b = (uMaxCorner - e1) / d;
    vec3 u = min(a, b);
    
    return max( max(-t, u.x), max(u.y, u.z) );
}

//----------------------------------------------------------------------
// Estimates the intensity gradient of the volume in model space
//----------------------------------------------------------------------

vec3 gradient(vec3 tc)
{
    vec3 nabla = vec3(
        texture3D(uVolume, tc + dx).r - texture3D(uVolume, tc - dx).r,
        texture3D(uVolume, tc + dy).r - texture3D(uVolume, tc - dy).r,
        texture3D(uVolume, tc + dz).r - texture3D(uVolume, tc - dz).r
    );
    
    return (nabla / uGradientDelta) * uTextureScale;
}


//----------------------------------------------------------------------
// Computes phong shading based on current light and material properties.
//----------------------------------------------------------------------

vec3 shade(vec3 p, vec3 v, vec3 n)
{
    vec4 lp = gl_ModelViewMatrixInverse * gl_LightSource[0].position;
    vec3 l = normalize(lp.xyz - p * lp.w);
    vec3 h = normalize(l+v);
    float cos_i = max(dot(n, l), 0.0);
    float cos_h = max(dot(n, h), 0.0);
    
    vec3 Ia = gl_FrontLightProduct[0].ambient.rgb;
    vec3 Id = gl_FrontLightProduct[0].diffuse.rgb * cos_i;
    vec3 Is = gl_FrontLightProduct[0].specular.rgb * pow(cos_h, gl_FrontMaterial.shininess);
    return (Ia + Id + Is);
}


//----------------------------------------------------------------------
// Main fragment shader code.
//----------------------------------------------------------------------

void main(void)
{
    vec4 camera = gl_ModelViewMatrixInverse * vec4(0.0, 0.0, 0.0, 1.0);
    vec3 raydir = normalize(vPosition.xyz - camera.xyz);
    
    //    float t_exit = exit(vPosition.xyz, raydir);
    float t_entry = entry(vPosition.xyz, raydir);
    t_entry = max(t_entry, -distance(camera.xyz, vPosition.xyz));
    
    // estimate a reasonable step size
    float t_step = distance(uMinCorner, uMaxCorner) / uResolution;
    vec3 tc_step = uTextureScale * (t_step * raydir);
    
    // cast the ray (in model space)
    vec4 sum = vec4(0.0);
    vec3 tc = gl_TexCoord[0].stp - tc_step * abs(t_entry) / t_step;
    
    for (float t = t_entry; t < 0.0; t += t_step, tc += tc_step)
    {
        // sample the volume for intensity (red channel)
        float intensity = texture3D(uVolume, tc).r;
        
        // look up intensity in the colour LUT
        vec4 colour = texture1D(uColorLUT, intensity);
        
        // skip empty space
        if (colour.a < 0.001) continue;
        
        // estimate gradient
        vec3 nabla = gradient(tc);
        
        // compute shading
        vec3 position = vPosition.xyz + t * raydir;
        vec3 normal = -normalize(nabla);
        vec3 view = -raydir;
        vec3 shaded = shade(position, view, normal);
        colour.rgb *= shaded;
        
        // compute transmission for this segment
        float Tr = exp(-colour.a * 400.0 * t_step);
        colour.rgb *= 1.0 - Tr;
        colour.a = 1.0 - Tr;
        
        // accumulate colour and opacity
        sum += (1.0 - sum.a) * colour;
        
        // early ray termination test
        if (sum.a > 0.96)
        {
            // calculate fragment depth
            vec4 clip = gl_ModelViewProjectionMatrix * vec4(position, 1.0);
            gl_FragDepth = (gl_DepthRange.diff * clip.z / clip.w +
                            gl_DepthRange.near + gl_DepthRange.far) * 0.5;
            
            break;
        }
    }
    
    gl_FragColor = sum;
}
