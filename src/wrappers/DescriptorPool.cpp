#include "DescriptorPool.hpp"

Vulking::DescriptorPool::DescriptorPool(
    const Device &device, std::vector<VkDescriptorPoolSize> &sizes,
    uint32_t maxSets)
    : device(device) {
  VkDescriptorPoolCreateInfo info{};
  info.sType = STYPE(DESCRIPTOR_POOL_CREATE_INFO);
  info.poolSizeCount = static_cast<uint32_t>(sizes.size());
  info.pPoolSizes = sizes.data();
  info.maxSets = maxSets;

  CHK(vkCreateDescriptorPool(device, &info, allocator, &pool),
      "failed to create descriptor pool");
}

Vulking::DescriptorPool::~DescriptorPool() {
  vkDestroyDescriptorPool(device, pool, allocator);
}

Vulking::DescriptorPool::operator VkDescriptorPool() const { return pool; }

std::vector<VkWriteDescriptorSet>
Vulking::DescriptorPool::allocateSets(const DescriptorSetLayout &layout,
                                  uint32_t count) {
  std::vector<VkDescriptorSetLayout> layouts(count, layout);
  VkDescriptorSetAllocateInfo info{};
  info.sType = STYPE(DESCRIPTOR_SET_ALLOCATE_INFO);
  info.descriptorPool = pool;
  info.descriptorSetCount = count;
  info.pSetLayouts = layouts.data();

  std::vector<VkDescriptorSet> sets(count);
  // These are deallocated when the set layout is deallocated.
  // This is fine if we don't need to change the sets.
  CHK(vkAllocateDescriptorSets(device, &info, sets.data()),
      "failed to allocate descriptor sets");

  std::vector<VkWriteDescriptorSet> writes(count);
  for (size_t i = 0; i < count; i++) {
    writes[i].sType = STYPE(WRITE_DESCRIPTOR_SET);
    writes[i].dstSet = sets[i];
  }

  return writes;
}
