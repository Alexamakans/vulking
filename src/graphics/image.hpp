#pragma once

#include <vulkan/vulkan_core.h>

class Image {
public:
  Image(int width, int height, VkFormat format);
  ~Image();

  operator VkImage() const { return image; }
  operator VkImageView() const { return view; }
  operator VkDeviceMemory() const { return memory; }

private:
  void createImage(uint32_t width, uint32_t height, uint32_t mipLevels,
                   VkSampleCountFlagBits numSamples, VkFormat format,
                   VkImageTiling tiling, VkImageUsageFlags usage,
                   VkMemoryPropertyFlags properties, VkImage &image,
                   VkDeviceMemory &imageMemory);

  void transitionImageLayout(VkImage image, VkFormat format,
                             VkImageLayout oldLayout, VkImageLayout newLayout,
                             uint32_t mipLevels);

  void generateMipmaps(VkImage image, VkFormat imageFormat, uint32_t width,
                       uint32_t height, uint32_t mipLevels);

  void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width,
                         uint32_t height);

  VkImage image;
  VkImageView view;
  VkDeviceMemory memory;
};
