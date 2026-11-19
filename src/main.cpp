#include <chrono>
#include <cmath>
#include <stdexcept>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <string>

#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>

#include "render/renderer.hpp"
#include "universe/galaxy.hpp"

namespace {

// Orbit camera around the galactic centre.
struct Camera {
    float yaw = 0.6f, pitch = 0.85f;
    float distance = 140'000.0f;  // ly
    double lastX = 0, lastY = 0;
    bool dragging = false;

    glm::vec3 position() const {
        return distance * glm::vec3(std::cos(pitch) * std::cos(yaw), std::sin(pitch), std::cos(pitch) * std::sin(yaw));
    }
};

Camera g_cam;
render::Renderer* g_renderer = nullptr;

void onMouseButton(GLFWwindow* w, int button, int action, int) {
    if (button != GLFW_MOUSE_BUTTON_LEFT) return;
    g_cam.dragging = action == GLFW_PRESS;
    glfwGetCursorPos(w, &g_cam.lastX, &g_cam.lastY);
}

void onCursor(GLFWwindow*, double x, double y) {
    if (!g_cam.dragging) return;
    g_cam.yaw += float(x - g_cam.lastX) * 0.005f;
    g_cam.pitch = glm::clamp(g_cam.pitch + float(y - g_cam.lastY) * 0.005f, -1.5f, 1.5f);
    g_cam.lastX = x;
    g_cam.lastY = y;
}

void onScroll(GLFWwindow*, double, double dy) {
    g_cam.distance = glm::clamp(g_cam.distance * std::pow(0.9f, float(dy)), 2'000.0f, 400'000.0f);
}

void onResize(GLFWwindow*, int, int) {
    if (g_renderer) g_renderer->onResize();
}

}  // namespace

int main(int argc, char** argv) {
    universe::GalaxyParams params;
    if (argc > 1) params.seed = std::strtoull(argv[1], nullptr, 0);
    if (argc > 2) params.starCount = static_cast<uint32_t>(std::strtoul(argv[2], nullptr, 0));
    // Headless check for CI / screenshots: render N frames, then exit.
    const char* framesEnv = std::getenv("STELLAR_FRAMES");
    const long maxFrames = framesEnv ? std::strtol(framesEnv, nullptr, 10) : -1;
    if (const char* d = std::getenv("STELLAR_DIST")) g_cam.distance = std::strtof(d, nullptr);
    if (const char* pt = std::getenv("STELLAR_PITCH")) g_cam.pitch = std::strtof(pt, nullptr);

    try {
        // Hand GLFW the loader we linked, so it does not search for libvulkan at runtime.
        glfwInitVulkanLoader(vkGetInstanceProcAddr);
        if (!glfwInit()) throw std::runtime_error("glfwInit failed");
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        GLFWwindow* window = glfwCreateWindow(1600, 900, "Stellar Engine", nullptr, nullptr);
        if (!window) throw std::runtime_error("glfwCreateWindow failed");
        glfwSetMouseButtonCallback(window, onMouseButton);
        glfwSetCursorPosCallback(window, onCursor);
        glfwSetScrollCallback(window, onScroll);
        glfwSetFramebufferSizeCallback(window, onResize);

        auto t0 = std::chrono::steady_clock::now();
        auto stars = universe::generateSpiralGalaxy(params);
        auto genMs = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t0).count();
        std::printf("Generated %zu stars (seed 0x%llX) in %.1f ms\n", stars.size(),
                    static_cast<unsigned long long>(params.seed), genMs);

        render::Renderer renderer(window, stars);
        g_renderer = &renderer;
        std::printf("GPU: %s\n", renderer.deviceName());

        auto fpsStart = std::chrono::steady_clock::now();
        int fpsFrames = 0;
        long frames = 0;
        while (!glfwWindowShouldClose(window) && (maxFrames < 0 || frames < maxFrames)) {
            glfwPollEvents();
            if (!g_cam.dragging) g_cam.yaw += 0.0008f;  // slow idle drift

            const glm::vec3 eye = g_cam.position();
            glm::mat4 view = glm::lookAt(eye, glm::vec3(0), glm::vec3(0, 1, 0));
            glm::mat4 proj = glm::perspective(glm::radians(55.0f), renderer.aspect(), 10.0f, 2'000'000.0f);
            proj[1][1] *= -1.0f;  // Vulkan clip space has Y pointing down
            renderer.drawFrame({proj * view, eye, 0.6f});

            ++frames;
            ++fpsFrames;
            auto now = std::chrono::steady_clock::now();
            double secs = std::chrono::duration<double>(now - fpsStart).count();
            if (secs >= 1.0) {
                char title[128];
                std::snprintf(title, sizeof(title), "Stellar Engine | %u stars | %.0f fps | %.1f ms",
                              params.starCount, fpsFrames / secs, 1000.0 * secs / fpsFrames);
                glfwSetWindowTitle(window, title);
                std::printf("%s\n", title);
                fpsStart = now;
                fpsFrames = 0;
            }
        }
        g_renderer = nullptr;
    } catch (const std::exception& e) {
        std::fprintf(stderr, "fatal: %s\n", e.what());
        glfwTerminate();
        return 1;
    }
    glfwTerminate();
    return 0;
}
