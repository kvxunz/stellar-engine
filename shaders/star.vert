#version 450

layout(location = 0) in vec3 inPos;
layout(location = 1) in float inSize;
layout(location = 2) in vec3 inColor;
layout(location = 3) in float inBrightness;

layout(push_constant) uniform Push {
    mat4 viewProj;
    vec4 cameraPos;   // xyz, w = exposure
} pc;

layout(location = 0) out vec3 vColor;

void main() {
    gl_Position = pc.viewProj * vec4(inPos, 1.0);
    float dist = max(length(inPos - pc.cameraPos.xyz), 1.0);
    // Inverse-square falloff, kept above a floor so far stars still read as dust.
    float atten = clamp(6.0e9 / (dist * dist), 0.04, 4.0);
    vColor = inColor * inBrightness * atten * pc.cameraPos.w;
    gl_PointSize = clamp(inSize * sqrt(atten) * 1.4, 1.0, 8.0);
}
