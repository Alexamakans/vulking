#pragma once

#include "../common.hpp"
#include "Device.hpp"
#include <vulkan/vulkan_core.h>

namespace Vulking {
class ImageView {
public:
  ImageView(const Device &device, VkImage image, VkFormat format,
            VkImageAspectFlagBits flags, uint32_t mipLevels);
  void release();
  operator VkImageView() const;

private:
  const Device &device;
  VkImageView view;

  VkImageView createImageView(VkImage image, VkFormat format,
                              VkImageAspectFlagBits flags, uint32_t mipLevels);
};
} // namespace Vulking
