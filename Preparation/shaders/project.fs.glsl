#version 120

varying vec3 v_Normal;

const vec3 LightDirection = vec3(1.0, -1.0, -1.0);

void main()
{
    vec3 N = normalize(v_Normal);
    vec3 L = normalize(-LightDirection);

    float LambertDiffuse = max(dot(N, L), 0.0);
    gl_FragColor = vec4(vec3(LambertDiffuse), 1.0);
}
