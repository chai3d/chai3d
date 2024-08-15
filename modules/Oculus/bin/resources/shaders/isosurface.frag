
// interpolated vertex position in model space (from vertex shader)
varying vec4 vPosition;

uniform vec3 uMinCorner;
uniform vec3 uMaxCorner;
uniform vec3 uTextureScale;
uniform vec3 uGradientDelta;

uniform sampler3D uVolume;
uniform float uIsosurface;

vec3 dx = vec3(uGradientDelta.x, 0.0, 0.0);
vec3 dy = vec3(0.0, uGradientDelta.y, 0.0);
vec3 dz = vec3(0.0, 0.0, uGradientDelta.z);

const float kMaxSamples = 400.0;

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

float entry(vec3 e1, vec3 d)
{
    float t = distance(uMinCorner, uMaxCorner);
    
    vec3 a = (uMinCorner - e1) / d;
    vec3 b = (uMaxCorner - e1) / d;
    vec3 u = min(a, b);
    
    return max( max(-t, u.x), max(u.y, u.z) );
}


// this function estimates the intensity gradient of the volume in model space
vec3 gradient(vec3 tc)
{
    vec3 nabla = vec3(
        texture3D(uVolume, tc + dx).r - texture3D(uVolume, tc - dx).r,
        texture3D(uVolume, tc + dy).r - texture3D(uVolume, tc - dy).r,
        texture3D(uVolume, tc + dz).r - texture3D(uVolume, tc - dz).r
    );
    return (nabla / uGradientDelta) * uTextureScale;
}

// Performs interval bisection and returns the value between a and b closest
// to isosurface. When s(b) > s(a), direction should be +1.0, and -1.0 otherwise.
vec3 refine(vec3 a, vec3 b, float isosurface, float direction)
{
    for (int i = 0; i < 6; ++i)
    {
        vec3 m = 0.5 * (a + b);
        float v = (texture3D(uVolume, m).r - isosurface) * direction;
        if (v >= 0.0)   b = m;
        else            a = m;
    }
    return b;
}

// computes phong shading based on current light and material properties
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

void main(void)
{
    vec4 camera = gl_ModelViewMatrixInverse * vec4(0.0, 0.0, 0.0, 1.0);
    vec3 raydir = normalize(vPosition.xyz - camera.xyz);
    
//    float t_exit = exit(vPosition.xyz, raydir);
    float t_entry = entry(vPosition.xyz, raydir);
    t_entry = max(t_entry, -distance(camera.xyz, vPosition.xyz));
    
    // estimate a reasonable step size
    float t_step = distance(uMinCorner, uMaxCorner) / kMaxSamples;
    vec3 tc_step = uTextureScale * (t_step * raydir);
    
    // cast the ray (in model space)
    vec4 sum = vec4(0.0);
    vec3 tc = gl_TexCoord[0].stp - tc_step * abs(t_entry) / t_step;
    
    for (float t = t_entry; t < 0.0; t += t_step, tc += tc_step)
    {
        // sample the volume for intensity (red channel)
        float intensity = texture3D(uVolume, tc).r;
        
        if (intensity > uIsosurface)
        {
            vec3 tcr = refine(tc - tc_step, tc, uIsosurface, 1.0);
            vec3 nabla = gradient(tcr);

            float dt = length(tcr - tc) / length(tc_step);
            vec3 position = vPosition.xyz + (t - dt * t_step) * raydir;
            vec3 normal = -normalize(nabla);
            vec3 view = -raydir;
            vec3 colour = shade(position, view, normal);

//            sum = vec4(normalize(nabla), 1.0);
            sum = vec4(colour, 1.0);
            
            // calculate fragment depth
            vec4 clip = gl_ModelViewProjectionMatrix * vec4(position, 1.0);
            gl_FragDepth = (gl_DepthRange.diff * clip.z / clip.w +
                            gl_DepthRange.near + gl_DepthRange.far) * 0.5;
            
            break;
        }
    }
    
    // discard the fragment if no geometry was intersected
    if (sum.a <= 0.0) discard;
    
    gl_FragColor = sum;
}
