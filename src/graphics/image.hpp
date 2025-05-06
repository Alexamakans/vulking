#pragma once

#include <cmath>
#include <vulkan/vulkan_core.h>

class Image {
public:
  Image(int width, int height, VkFormat format, VkImageUsageFlagBits usage,
        int mipLevels, VkSampleCountFlagBits msaaSamples,
        VkImageAspectFlagBits flags);
  ~Image();

  operator VkImage() const { return image; }
  operator VkImage *() { return &image; }
  operator VkImageView() const { return view; }
  operator VkImageView *() { return &view; }
  operator VkDeviceMemory() const { return memory; }
  operator VkDeviceMemory *() { return &memory; }

  static uint32_t calculateMipLevels(int width, int height);
  static bool hasStencilComponent(VkFormat format);

private:
  void createImage(uint32_t width, uint32_t height, uint32_t mipLevels,
                   VkSampleCountFlagBits numSamples, VkFormat format,
                   VkImageTiling tiling, VkImageUsageFlags usage,
                   VkMemoryPropertyFlags properties, VkImage &image,
                   VkDeviceMemory &imageMemory);

  VkImageView createImageView(VkImage image, VkFormat format,
                              VkImageAspectFlags aspectFlags,
                              uint32_t mipLevels);

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
