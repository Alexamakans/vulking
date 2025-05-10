#pragma once

#include "../common.hpp"
#include "DescriptorSetLayout.hpp"
#include "Device.hpp"
#include <vulkan/vulkan_core.h>

namespace Vulking {
class DescriptorPool {
public:
  DescriptorPool(const Device &device, std::vector<VkDescriptorPoolSize> &sizes,
                 uint32_t maxSets);
  ~DescriptorPool();
  operator VkDescriptorPool() const;

  std::vector<VkWriteDescriptorSet>
  allocateSets(const DescriptorSetLayout &layout, uint32_t count);

private:
  const Device &device;
  VkDescriptorPool pool;
};
} // namespace Vulking
