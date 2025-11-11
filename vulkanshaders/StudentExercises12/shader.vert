#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec4 color;

layout(location = 0) out vec4 interColor;
layout(location = 1) out vec4 interPos;

layout(push_constant) uniform PushConstants {
    mat4 modelMat;
} pc;

layout(set = 0, binding = 0) uniform UBOVertex {
    mat4 viewMat;
    mat4 projMat;
} ubo;

void main() {
    vec4 pos = vec4(position, 1.0);
    //pos = ubo.projMat * ubo.viewMat * pc.modelMat * pos;
    vec4 viewPos = ubo.viewMat * pc.modelMat * pos;
    interPos = viewPos;

    gl_Position = ubo.projMat * viewPos;

    interColor = color;

}