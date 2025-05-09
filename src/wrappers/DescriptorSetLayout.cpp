#include "DescriptorSetLayout.hpp"
Vulking::DescriptorSetLayout::DescriptorSetLayout(
    const Device &device, std::vector<VkDescriptorSetLayoutBinding> bindings)
    : device(device) {
  VkDescriptorSetLayoutCreateInfo info{};
  info.sType = STYPE(DESCRIPTOR_SET_LAYOUT_CREATE_INFO);
  info.bindingCount = static_cast<uint32_t>(bindings.size());
  info.pBindings = bindings.data();
  CHK(vkCreateDescriptorSetLayout(device, &info, allocator, &layout),
      "failed to create descriptor set layout");
};

void Vulking::DescriptorSetLayout::release() {
  vkDestroyDescriptorSetLayout(device, layout, allocator);
}
