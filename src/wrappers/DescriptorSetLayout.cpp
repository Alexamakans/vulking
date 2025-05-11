#include "DescriptorSetLayout.hpp"
#include <vulkan/vulkan_core.h>

Vulking::DescriptorSetLayout::DescriptorSetLayout(
    const Device &device, std::vector<VkDescriptorSetLayoutBinding> bindings)
    : device(device), bindingCount(static_cast<uint32_t>(bindings.size())) {
  VkDescriptorSetLayoutCreateInfo info{};
  info.sType = STYPE(DESCRIPTOR_SET_LAYOUT_CREATE_INFO);
  info.bindingCount = bindingCount;
  info.pBindings = bindings.data();
  CHK(vkCreateDescriptorSetLayout(device, &info, allocator, &layout),
      "failed to create descriptor set layout");
};

void Vulking::DescriptorSetLayout::release() {
  vkDestroyDescriptorSetLayout(device, layout, allocator);
}
