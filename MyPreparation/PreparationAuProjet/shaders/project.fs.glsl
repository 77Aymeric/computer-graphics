#version 120

varying vec3 v_Normal;
varying vec3 v_WorldPos;

uniform vec3 u_LightDir;  // direction TO the light (world)
uniform vec3 u_BaseColor;

void main()
{
    vec3 N = normalize(v_Normal);
    vec3 L = normalize(u_LightDir);

    float ndotl = max(dot(N, L), 0.0);
    vec3 diffuse = ndotl * u_BaseColor;
    vec3 ambient = 0.15 * u_BaseColor;

    gl_FragColor = vec4(diffuse + ambient, 1.0);
}

