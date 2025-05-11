#pragma once

#include "../common.hpp"

#include "../wrappers/CommandPool.hpp"
#include "../wrappers/DescriptorPool.hpp"
#include "../wrappers/Device.hpp"
#include "../wrappers/PhysicalDevice.hpp"
#include "../wrappers/Queue.hpp"
#include "../wrappers/RenderPass.hpp"
#include <vulkan/vulkan_core.h>

namespace Vulking {
class GPU {
public:
  GPU(const Instance &instance, const Surface &surface);

  void release() {
    if (descriptorPool != nullptr) {
      std::destroy_at(descriptorPool);
      descriptorPool = nullptr;
    }
    commandPool.release();
    renderPass.release();
    device.release();
  }

  uint32_t getGraphicsQueueFamily() const;
  const Queue &getGraphicsQueue() const { return graphicsQueue; }

  uint32_t getPresentQueueFamily() const;
  const Queue &getPresentQueue() const { return presentQueue; }

  const RenderPass &getRenderPass() const;
  const CommandPool &getCommandPool() const;
  const DescriptorPool &getDescriptorPool() const;
  void setDescriptorPool(std::vector<VkDescriptorPoolSize> &sizes,
                         uint32_t maxSets) {
    if (descriptorPool == nullptr) {
      descriptorPool =
          new (descriptorPoolBuf) DescriptorPool(device, sizes, maxSets);
    } else {
      descriptorPool->release();
      descriptorPool->create(device, sizes, maxSets);
    }
  }

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
  const CommandPool commandPool;
  alignas(DescriptorPool) std::byte descriptorPoolBuf[sizeof(DescriptorPool)];
  DescriptorPool *descriptorPool = nullptr;

  VkRenderPass createRenderPass();
};
} // namespace Vulking
