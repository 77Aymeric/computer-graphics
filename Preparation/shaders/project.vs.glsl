#version 120

attribute vec3 a_Position;
attribute vec3 a_Normal;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;
uniform mat3 u_NormalMatrix;

varying vec3 v_Normal;

void main()
{
    vec4 worldPos = u_Model * vec4(a_Position, 1.0);

    gl_Position = u_Projection * u_View * worldPos;

    v_Normal = normalize(u_NormalMatrix * a_Normal);
}
