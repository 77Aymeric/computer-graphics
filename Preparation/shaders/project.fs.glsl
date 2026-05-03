#version 120

varying vec3 v_Normal;

const vec3 LightDirection = vec3(1.0, -1.0, -1.0);

void main()
{
    vec3 N = normalize(v_Normal);
    vec3 L = normalize(-LightDirection);

    float LambertDiffuse = max(dot(N, L), 0.0);

    float ambientStrength = 0.15;
    float lighting = clamp(ambientStrength + LambertDiffuse, 0.0, 1.0);

    gl_FragColor = vec4(vec3(lighting), 1.0);
}
