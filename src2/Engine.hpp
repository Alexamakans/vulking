#pragma once

#include "Buffer.hpp"
#include "Common.hpp"
#include "Device.hpp"
#include "PhysicalDevice.hpp"
#include <vulkan/vulkan_core.h>

#define VULKING_MAKE_VERSION(major, minor, patch)                              \
  VK_MAKE_VERSION(major, minor, patch)

using Instance = VkInstance;
using Queue = VkQueue;
using Sampler = VkSampler;
using Fence = VkFence;
using Semaphore = VkSemaphore;
using CommandBuffer = VkCommandBuffer;
using Image = VkImage;
using ImageView = VkImageView;
using Swapchain = VkSwapchainKHR;
using Pipeline = VkPipeline;

// ShaderModule can be released after the creation of the pipelines it's used
// in.
using ShaderModule = VkShaderModule;

namespace Vulking {
class Engine {
public:
  Engine(GLFWwindow *window, const char *applicationInfo,
         uint32_t applicationVersion,
         std::vector<const char *> &requiredExtensions);

  ~Engine() {
    device.destroy();
    vkDestroyInstance(instance, ALLOCATOR);
  }

  Sampler createSampler();
  Buffer createBuffer();
  Image createImage();
  ImageView createImageView();
  CommandBuffer createCommandBuffer();

  // Pipeline is so huge that it should have a builder util or something
  // probably, with sane defaults.
  Pipeline createPipeline();

private:
  VkInstance createInstance(const char *applicationInfo,
                            uint32_t applicationVersion,
                            std::vector<const char *> &requiredExtensions);

  VkInstance instance;
  VkSurfaceKHR surface;

  PhysicalDevice physicalDevice;
  Device device;
  VkCommandPool commandPool;
  VkDescriptorPool descriptorPool;

  uint32_t graphicsQueueFamily;
  uint32_t presentQueueFamily;

  VkPhysicalDevice getSuitablePhysicalDevice(VkInstance instance);
  bool isDeviceSuitable(VkPhysicalDevice physicalDevice) const;
};
} // namespace Vulking
