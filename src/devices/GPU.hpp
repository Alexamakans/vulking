#pragma once

#include "../common.hpp"

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

  operator PhysicalDevice() const { return physicalDevice; }
  operator Device() const { return device; }

  uint32_t getGraphicsQueueFamily() const;
  uint32_t getPresentQueueFamily() const;
  const RenderPass &getRenderPass() const;

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

  VkRenderPass createRenderPass();
};
} // namespace Vulking
