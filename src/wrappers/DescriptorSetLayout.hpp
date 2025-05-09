#pragma once

#include "../common.hpp"
#include "Device.hpp"
#include <vulkan/vulkan_core.h>

namespace Vulking {
class DescriptorSetLayout {
public:
  DescriptorSetLayout(const Device &device,
                      std::vector<VkDescriptorSetLayoutBinding> bindings);

  void release();

  operator VkDescriptorSetLayout() const { return layout; }

private:
  const Device &device;

  VkDescriptorSetLayout layout;
};
} // namespace Vulking
