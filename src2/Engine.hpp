#pragma once

#include "Common.hpp"

#include "Buffer.hpp"
#include <vulkan/vulkan.hpp>

#define VULKING_MAKE_VERSION(major, minor, patch)                              \
  VK_MAKE_VERSION(major, minor, patch)

using Instance = vk::Instance;
using Queue = vk::Queue;
using Sampler = vk::Sampler;
using Fence = vk::Fence;
using Semaphore = vk::Semaphore;
using CommandBuffer = vk::CommandBuffer;
using Swapchain = vk::SwapchainKHR;
using Pipeline = vk::Pipeline;

// ShaderModule can be released after the creation of the pipelines it's used
// in.
using ShaderModule = vk::ShaderModule;

namespace Vulking {
class Engine {
public:
  Engine(GLFWwindow *window, const char *applicationInfo,
         uint32_t applicationVersion,
         const std::vector<const char *> &requiredExtensions);

  ~Engine();

  vk::SamplerCreateInfo &defaultSamplerInfo();
  Sampler createSampler(const vk::SamplerCreateInfo &info);

  Buffer createBuffer(void *src, vk::DeviceSize size,
                      vk::BufferUsageFlags usage,
                      vk::MemoryPropertyFlags properties,
                      const char *name = "unnamed");

  CommandBuffer createCommandBuffer();

  // TODO: Pipeline is so huge that it should have a builder util or something
  // probably, with sane defaults.
  //
  // Look at ImageBuilder for inspiration?
  Pipeline createPipeline();

  static vk::UniqueInstance instance;
  static vk::PhysicalDevice physicalDevice;
  static vk::UniqueDevice device;

private:
  vk::UniqueInstance
  createInstance(const char *applicationInfo, uint32_t applicationVersion,
                 const std::vector<const char *> &requiredExtensions);

  vk::UniqueDevice createDevice();

  vk::SurfaceKHR surface;

  vk::CommandPool commandPool;
  vk::DescriptorPool descriptorPool;

  uint32_t graphicsQueueFamily;
  uint32_t presentQueueFamily;

  vk::PhysicalDevice getSuitablePhysicalDevice();
  bool isDeviceSuitable(vk::PhysicalDevice physicalDevice) const;
};
} // namespace Vulking
