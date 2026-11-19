#version 450

layout(location = 0) in vec3 vColor;
layout(location = 0) out vec4 outColor;

void main() {
    // Soft round sprite: Gaussian core plus a faint halo.
    vec2 d = gl_PointCoord * 2.0 - 1.0;
    float r2 = dot(d, d);
    if (r2 > 1.0) discard;
    float core = exp(-r2 * 6.0);
    float halo = 0.15 * (1.0 - r2);
    outColor = vec4(vColor * (core + halo), 1.0);
}
