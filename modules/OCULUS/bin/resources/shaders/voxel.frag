
// interpolated vertex position in model space (from vertex shader)
varying vec4 vPosition;

uniform vec3 uMinCorner;
uniform vec3 uMaxCorner;
uniform vec3 uTextureScale;
uniform sampler3D uVolume;

const float kMaxSamples = 500.0;

// this function finds the exiting intersection between a ray e0+d and
// the volume's bounding box
float exit(vec3 e0, vec3 d)
{
    float t = distance(uMinCorner, uMaxCorner);
    
    vec3 a = (uMinCorner - e0) / d;
    vec3 b = (uMaxCorner - e0) / d;
    vec3 u = max(a, b);
    
    return min( min(t, u.x), min(u.y, u.z) );
}

void main(void)
{
    vec4 camera = gl_ModelViewMatrixInverse * vec4(0.0, 0.0, 0.0, 1.0);
    vec3 raydir = normalize(vPosition.xyz - camera.xyz);
    
    float t_exit = exit(vPosition.xyz, raydir);
    
    // estimate a reasonable step size
    float t_step = distance(uMinCorner, uMaxCorner) / kMaxSamples;
    vec3 tc_step = uTextureScale * (t_step * raydir);
    
    // cast the ray (in model space)
    vec4 sum = vec4(0.0);
    vec3 tc = gl_TexCoord[0].stp;
    for (float t = 0.0; t < t_exit; t += t_step, tc += tc_step)
    {
        // sample the texture for the colour
        vec4 colour = texture3D(uVolume, tc);
        
        // compute transmission for this segment
        float sigma = 10.0;  // optical density
        float Tr = exp(-sigma * t_step);
        colour *= 1.0 - Tr;
        
        // accumulate colour and opacity
        sum += (1.0 - sum.a) * colour;
    }
    
    gl_FragColor = sum;
}
