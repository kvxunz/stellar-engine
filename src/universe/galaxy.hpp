#pragma once
#include <cstdint>
#include <vector>
#include <glm/glm.hpp>

namespace universe {

// One star as the GPU sees it. Layout matches shaders/star.vert.
struct Star {
    glm::vec3 position;  // light-years, galaxy centre at origin
    float     size;      // point size hint
    glm::vec3 color;     // linear RGB from temperature
    float     brightness;
};

struct GalaxyParams {
    uint64_t seed        = 0x5EED'C0DEull;
    uint32_t starCount   = 200'000;
    uint32_t arms        = 4;
    float    radius      = 50'000.0f;  // ly
    float    thickness   = 1'000.0f;   // ly, disc scale height
    float    armTwist    = 4.5f;       // radians of winding from centre to rim
    float    armSpread   = 0.22f;      // angular scatter around an arm
    float    bulgeFrac   = 0.18f;      // share of stars in the central bulge
};

// Deterministic: the same params always give the same galaxy.
std::vector<Star> generateSpiralGalaxy(const GalaxyParams& p);

// Approximate blackbody colour for 1000 K .. 40000 K.
glm::vec3 temperatureToRgb(float kelvin);

}  // namespace universe
