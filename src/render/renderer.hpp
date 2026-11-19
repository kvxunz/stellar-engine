#pragma once
#include <array>
#include <vector>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "universe/galaxy.hpp"

namespace render {

struct FrameParams {
    glm::mat4 viewProj;
    glm::vec3 cameraPos;
    float     exposure;
};

// Minimal Vulkan 1.3 renderer: dynamic rendering, no render passes,
// two frames in flight, one point-sprite pipeline for the star field.
class Renderer {
public:
    Renderer(GLFWwindow* window, const std::vector<universe::Star>& stars);
    ~Renderer();
    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    void drawFrame(const FrameParams& params);
    void onResize() { resized_ = true; }
    float aspect() const { return float(extent_.width) / float(extent_.height); }
    const char* deviceName() const { return deviceName_; }

private:
    static constexpr uint32_t kFramesInFlight = 2;

    void createInstance();
    void pickDevice();
    void createSwapchain();
    void destroySwapchain();
    void recreateSwapchain();
    void createPipeline();
    void uploadStars(const std::vector<universe::Star>& stars);
    void record(VkCommandBuffer cmd, uint32_t imageIndex, const FrameParams& p);
    uint32_t findMemoryType(uint32_t bits, VkMemoryPropertyFlags flags) const;

    GLFWwindow*      window_;
    VkInstance       instance_ = VK_NULL_HANDLE;
    VkSurfaceKHR     surface_ = VK_NULL_HANDLE;
    VkPhysicalDevice gpu_ = VK_NULL_HANDLE;
    VkDevice         device_ = VK_NULL_HANDLE;
    VkQueue          queue_ = VK_NULL_HANDLE;
    uint32_t         queueFamily_ = 0;
    char             deviceName_[VK_MAX_PHYSICAL_DEVICE_NAME_SIZE] = {};

    VkSwapchainKHR           swapchain_ = VK_NULL_HANDLE;
    VkFormat                 format_ = VK_FORMAT_UNDEFINED;
    VkExtent2D               extent_{};
    std::vector<VkImage>     images_;
    std::vector<VkImageView> views_;
    std::vector<VkSemaphore> renderDone_;  // one per swapchain image
    bool                     resized_ = false;

    VkPipelineLayout layout_ = VK_NULL_HANDLE;
    VkPipeline       pipeline_ = VK_NULL_HANDLE;

    VkBuffer       starBuffer_ = VK_NULL_HANDLE;
    VkDeviceMemory starMemory_ = VK_NULL_HANDLE;
    uint32_t       starCount_ = 0;

    VkCommandPool commandPool_ = VK_NULL_HANDLE;
    std::array<VkCommandBuffer, kFramesInFlight> cmds_{};
    std::array<VkSemaphore, kFramesInFlight>     imageReady_{};
    std::array<VkFence, kFramesInFlight>         inFlight_{};
    uint32_t frame_ = 0;
};

}  // namespace render
