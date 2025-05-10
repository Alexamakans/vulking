#pragma once

#include "../common.hpp"

#include "../wrappers/CommandPool.hpp"
#include "../wrappers/Device.hpp"
#include "../wrappers/PhysicalDevice.hpp"
#include "../wrappers/Queue.hpp"
#include "../wrappers/RenderPass.hpp"
#include <vulkan/vulkan_core.h>

namespace Vulking {
class GPU {
public:
  GPU(const Instance &instance, const Surface &surface);

  void release() { device.release(); }

  uint32_t getGraphicsQueueFamily() const;
  uint32_t getPresentQueueFamily() const;
  const RenderPass &getRenderPass() const;
  const RenderPass &getCommandPool() const;

  const PhysicalDevice physicalDevice;
  const Device device;

private:
  const Instance &instance;
  const Surface &surface;

  const Queue graphicsQueue;
  const Queue presentQueue;

  uint32_t graphicsQueueFamily{};
  uint32_t presentQueueFamily{};

  const RenderPass renderPass;
  const CommandPool commandPool;

  VkRenderPass createRenderPass();
};
} // namespace Vulking
