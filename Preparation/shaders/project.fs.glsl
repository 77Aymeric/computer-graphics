#version 120

varying vec3 v_Normal;

const vec3 LightDirection = vec3(1.0, -1.0, -1.0);

void main()
{
    vec3 N = normalize(v_Normal);
    vec3 L = normalize(-LightDirection);

    float LambertDiffuse = max(dot(N, L), 0.0);

    //hemispheric et gamma 
    vec3 up = vec3(0.0, 1.0, 0.0);
    float t = 0.5 * (dot(N, up) + 1.0); // 0 = down-facing, 1 = up-facing

    vec3 skyColor = vec3(0.25, 0.35, 0.55);
    vec3 groundColor = vec3(0.20, 0.18, 0.15);
    vec3 hemiAmbient = mix(groundColor, skyColor, t);

    vec3 colorLinear = clamp(hemiAmbient + vec3(LambertDiffuse), 0.0, 1.0);
    vec3 colorSRGB = pow(colorLinear, vec3(1.0 / 2.2));

    gl_FragColor = vec4(colorSRGB, 1.0);
}
