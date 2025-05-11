#include "DescriptorPool.hpp"

Vulking::DescriptorPool::DescriptorPool(
    const Device &device, std::vector<VkDescriptorPoolSize> &sizes,
    uint32_t maxSets)
    : device(device) {
  create(device, sizes, maxSets);
}

void Vulking::DescriptorPool::create(const Device &device,
                                     std::vector<VkDescriptorPoolSize> &sizes,
                                     uint32_t maxSets) {
  VkDescriptorPoolCreateInfo info{};
  info.sType = STYPE(DESCRIPTOR_POOL_CREATE_INFO);
  info.poolSizeCount = static_cast<uint32_t>(sizes.size());
  info.pPoolSizes = sizes.data();
  info.maxSets = maxSets;

  CHK(vkCreateDescriptorPool(device, &info, allocator, &pool),
      "failed to create descriptor pool");
}

Vulking::DescriptorPool::~DescriptorPool() { release(); }

void Vulking::DescriptorPool::release() {
  if (pool != VK_NULL_HANDLE) {
    vkDestroyDescriptorPool(device, pool, allocator);
    pool = VK_NULL_HANDLE;
  }
}

Vulking::DescriptorPool::operator VkDescriptorPool() const { return pool; }

std::vector<std::vector<VkWriteDescriptorSet>>
Vulking::DescriptorPool::allocateSets(
    const DescriptorSetLayout &layout, uint32_t count,
    std::vector<VkDescriptorSet> &sets) const {
  std::vector<VkDescriptorSetLayout> layouts(count, layout);
  VkDescriptorSetAllocateInfo info{};
  info.sType = STYPE(DESCRIPTOR_SET_ALLOCATE_INFO);
  info.descriptorPool = pool;
  info.descriptorSetCount = count;
  info.pSetLayouts = layouts.data();

  // These are deallocated when the set layout is deallocated.
  // This is fine if we don't need to change the sets.
  CHK(vkAllocateDescriptorSets(device, &info, sets.data()),
      "failed to allocate descriptor sets");

  std::vector<std::vector<VkWriteDescriptorSet>> allWrites;
  for (uint32_t i = 0; i < count; i++) {
    std::vector<VkWriteDescriptorSet> writes(layout.bindingCount);
    for (auto &write : writes) {
      write.sType = STYPE(WRITE_DESCRIPTOR_SET);
      write.dstSet = sets[i];
    }

    allWrites.emplace_back(writes);
  }

  return allWrites;
}
