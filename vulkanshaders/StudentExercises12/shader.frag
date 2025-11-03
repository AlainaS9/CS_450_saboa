#version 450

layout(location = 0) in vec4 interColor;

layout(location = 0) out vec4 out_color;

void main() {
    out_color = interColor;
}