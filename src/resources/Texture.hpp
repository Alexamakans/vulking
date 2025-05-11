#pragma once

#include "../common.hpp"
#include "../wrappers/CommandPool.hpp"
#include "../wrappers/Device.hpp"
#include "../wrappers/PhysicalDevice.hpp"
#include "../wrappers/Queue.hpp"
#include <vulkan/vulkan_core.h>

namespace Vulking {
class Texture {
public:
  Texture(const PhysicalDevice &physicalDevice, const Device &device,
          const CommandPool &commandPool, const Queue &queue,
          const std::string &path);

  void release() {
    vkDestroyImageView(device, view, allocator);
    vkDestroyImage(device, image, allocator);
    vkFreeMemory(device, memory, allocator);
    vkDestroySampler(device, sampler, allocator);
  }

  VkImageView getView() const { return view; }
  VkSampler getSampler() const { return sampler; }

private:
  const PhysicalDevice &physicalDevice;
  const Device &device;

  int width, height, channels;
  uint32_t mipLevels;

  VkImage image;
  VkDeviceMemory memory;
  VkImageView view;
  // TODO: Sampler definitely shouldn't be per texture
  VkSampler sampler;

  void createSampler(const PhysicalDevice &physicalDevice,
                     const Device &device);
};
} // namespace Vulking
