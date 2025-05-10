#pragma once

#include "../common.hpp"
#include "../wrappers/CommandPool.hpp"
#include "../wrappers/Device.hpp"
#include "../wrappers/PhysicalDevice.hpp"
#include "../wrappers/Queue.hpp"

namespace Vulking {
class Texture {
public:
  Texture(const PhysicalDevice &physicalDevice, const Device &device,
          const CommandPool &commandPool, const Queue &queue,
          const std::string &path);

private:
  const PhysicalDevice &physicalDevice;
  const Device &device;

  int width, height, channels;
  uint32_t mipLevels;

  VkImage image;
  VkDeviceMemory memory;
  VkImageView view;
  VkSampler sampler;

  void createSampler(const PhysicalDevice &physicalDevice,
                     const Device &device);
};
} // namespace Vulking
