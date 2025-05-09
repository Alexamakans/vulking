#pragma once

#include "../common.hpp"
#include "Device.hpp"
#include <vulkan/vulkan_core.h>

namespace Vulking {
class GraphicsPipeline {
public:
  GraphicsPipeline(const Device &device);

  void release() { vkDestroyPipeline(device, pipeline, allocator); };

  operator VkPipeline() const { return pipeline; }

private:
  const Device &device;
  VkPipeline pipeline;
};
} // namespace Vulking
