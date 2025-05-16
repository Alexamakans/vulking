#pragma once

#include "Common.hpp"
#include "Device.hpp"
#include "Image.hpp"

namespace Vulking {
// ImageBuilder currently only supports 2D images.
class ImageBuilder {
public:
  ImageBuilder &size(uint32_t width, uint32_t height);
  ImageBuilder setMipLevels(uint32_t mipLevels);
  // CalculateMipLevels MUST be called after Size.
  ImageBuilder calculateMipLevels();
  ImageBuilder &usage(VkImageUsageFlags usage);
  ImageBuilder &memoryProperties(VkMemoryPropertyFlags properties);
  ImageBuilder &multisamples(const Device &device);
  ImageBuilder &multisamples(VkSampleCountFlagBits samples);
  ImageBuilder &format(VkFormat format);
  ImageBuilder &flags(VkImageCreateFlags flags);
  ImageBuilder &imageType(VkImageType imageType);
  ImageBuilder &arrayLayers(uint32_t arrayLayers);
  ImageBuilder &tiling(VkImageTiling tiling);
  ImageBuilder &sharingMode(VkSharingMode sharingMode);
  ImageBuilder &queueFamilyIndexCount(uint32_t queueFamilyIndexCount);
  ImageBuilder &pQueueFamilyIndices(const uint32_t *pQueueFamilyIndices);
  ImageBuilder &initialLayout(VkImageLayout initialLayout);

  Image create(const Device &device, const char *name = "unnamed");

private:
  VkImageCreateInfo info{
      .sType = STYPE(IMAGE_CREATE_INFO),
      .imageType = VK_IMAGE_TYPE_2D,
      .format = VK_FORMAT_R8G8B8A8_SRGB,
      .mipLevels = 1,
      .arrayLayers = 1,
      .samples = VK_SAMPLE_COUNT_1_BIT,
      .tiling = VK_IMAGE_TILING_OPTIMAL,
      .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
      .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
  };
  VkMemoryPropertyFlags properties = 0;
};
} // namespace Vulking
