#version 450

layout(location = 0) in vec4 interColor;
layout(location = 1) in vec4 interPos;
layout(location = 2) in vec3 interNormal;

layout(location = 0) out vec4 out_color;

void main() {
    vec3 N = normalize(interNormal);

    //out_color = interColor;
    out_color = vec4(N, 1.0);
}