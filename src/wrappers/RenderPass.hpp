#pragma once

#include "../common.hpp"
#include "Device.hpp"
#include <vulkan/vulkan_core.h>

namespace Vulking {

class RenderPass {
public:
  RenderPass(const Device &device, VkFormat colorFormat, VkFormat depthFormat,
             VkSampleCountFlagBits msaaSamples);

  void release() const;

  operator VkRenderPass() const { return renderPass; }

private:
  const Device &device;
  VkFormat colorFormat;
  VkFormat depthFormat;
  VkSampleCountFlagBits msaaSamples;

  VkRenderPass renderPass;

  VkRenderPass createRenderPass();
};
} // namespace Vulking
