#include "universe/galaxy.hpp"

#include <algorithm>
#include <cmath>
#include <random>

namespace universe {

glm::vec3 temperatureToRgb(float kelvin) {
    // Tanner Helland's fit, returned as linear 0..1.
    const float t = std::clamp(kelvin, 1000.0f, 40000.0f) / 100.0f;
    float r, g, b;
    if (t <= 66.0f) {
        r = 255.0f;
        g = 99.4708025861f * std::log(t) - 161.1195681661f;
        b = t <= 19.0f ? 0.0f : 138.5177312231f * std::log(t - 10.0f) - 305.0447927307f;
    } else {
        r = 329.698727446f * std::pow(t - 60.0f, -0.1332047592f);
        g = 288.1221695283f * std::pow(t - 60.0f, -0.0755148492f);
        b = 255.0f;
    }
    auto norm = [](float v) { return std::pow(std::clamp(v, 0.0f, 255.0f) / 255.0f, 2.2f); };
    return {norm(r), norm(g), norm(b)};
}

namespace {

// Rough main-sequence mix: many cool red dwarfs, few hot blue giants.
float sampleTemperature(std::mt19937_64& rng, bool inBulge) {
    std::uniform_real_distribution<float> u(0.0f, 1.0f);
    const float x = u(rng);
    if (inBulge) return 2800.0f + 3500.0f * x;  // older, redder population
    if (x < 0.70f) return 2600.0f + 1200.0f * u(rng);   // M
    if (x < 0.88f) return 3800.0f + 1500.0f * u(rng);   // K
    if (x < 0.96f) return 5300.0f + 1000.0f * u(rng);   // G
    if (x < 0.99f) return 6300.0f + 3500.0f * u(rng);   // F/A
    return 10000.0f + 25000.0f * u(rng);                // B/O
}

}  // namespace

std::vector<Star> generateSpiralGalaxy(const GalaxyParams& p) {
    std::mt19937_64 rng(p.seed);
    std::uniform_real_distribution<float> u(0.0f, 1.0f);
    std::normal_distribution<float> n(0.0f, 1.0f);
    constexpr float kTau = 6.28318530718f;

    std::vector<Star> stars;
    stars.reserve(p.starCount);

    for (uint32_t i = 0; i < p.starCount; ++i) {
        Star s{};
        const bool bulge = u(rng) < p.bulgeFrac;

        if (bulge) {
            // Triaxial Gaussian bulge.
            const float r = p.radius * 0.12f;
            s.position = {n(rng) * r, n(rng) * r * 0.6f, n(rng) * r * 0.8f};
        } else {
            // Exponential disc radius, then wrap onto a logarithmic spiral arm.
            // Resample instead of clamping, so no stars pile up on the rim.
            float rad;
            do { rad = -std::log(1.0f - u(rng)) * p.radius * 0.28f; } while (rad > p.radius);
            const uint32_t arm = static_cast<uint32_t>(u(rng) * p.arms) % p.arms;
            const float base = kTau * static_cast<float>(arm) / static_cast<float>(p.arms);
            const float twist = p.armTwist * (rad / p.radius);
            const float scatter = n(rng) * p.armSpread * (1.2f - rad / p.radius);
            const float angle = base + twist + scatter;
            const float height = n(rng) * p.thickness * std::exp(-rad / p.radius);
            s.position = {std::cos(angle) * rad, height, std::sin(angle) * rad};
        }

        const float temp = sampleTemperature(rng, bulge);
        s.color = temperatureToRgb(temp);
        // Hot stars are brighter; add scatter so the field is not uniform.
        s.brightness = std::clamp(0.25f + temp / 12000.0f + 0.2f * n(rng), 0.08f, 2.5f);
        s.size = 1.0f + std::min(temp / 8000.0f, 3.0f);
        stars.push_back(s);
    }
    return stars;
}

}  // namespace universe
