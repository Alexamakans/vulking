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
  void create(const Device &device, std::vector<VkDescriptorPoolSize> &sizes,
              uint32_t maxSets);
  ~DescriptorPool();
  void release();
  operator VkDescriptorPool() const;

  std::vector<std::vector<VkWriteDescriptorSet>>
  allocateSets(const DescriptorSetLayout &layout, uint32_t count,
               std::vector<VkDescriptorSet> &sets) const;

private:
  const Device &device;
  VkDescriptorPool pool = VK_NULL_HANDLE;
};
} // namespace Vulking
