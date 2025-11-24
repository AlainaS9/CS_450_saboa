#version 450

layout(location = 0) in vec4 interColor;
layout(location = 1) in vec4 interPos;
layout(location = 2) in vec3 interNormal;

layout(location = 0) out vec4 out_color;

struct PointLight {
    vec4 pos;
    vec4 vpos;
    vec4 color;
};

layout(set = 0, binding = 1) uniform UBOFragment {
    PointLight light;
} ubo;

void main() {
    vec3 N = normalize(interNormal);

    vec3 L = vec3(ubo.light.vpos - interPos);
    float d = length(L);
    float at = (1.0 / (d*d + 1.0));
    L = normalize(L);

    float diffCoef = max(0, dot(N, L));

    vec3 diffColor = diffCoef*vec3(interColor)*vec3(ubo.light.color);

    vec3 V = vec3(-interPos);
    vec3 H = normalize(V + L);

    float shine = 10.0;
    float specCoef = pow(max(0, dot(H, N)), shine);
    vec3 specColor = diffCoef*specCoef*vec3(ubo.light.color);

    vec3 finalColor = diffColor + specColor;

    //out_color = interColor;
    //out_color = vec4(N, 1.0);
    //out_color = ubo.light.vpos;
    //out_color = vec4(at, at, at, 1.0);
    //out_color = vec4(specCoef, specCoef, specCoef, 1.0);
    out_color = vec4(finalColor, 1.0);
}