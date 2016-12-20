#version 130

out vec3 N;
out vec3 v;
out vec2 vTexCoord;

void main(void)  
{     
   vTexCoord = gl_MultiTexCoord0.xy;
   v = vec3(gl_ModelViewMatrix * gl_Vertex);
   N = normalize(gl_NormalMatrix * gl_Normal);
   gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}
