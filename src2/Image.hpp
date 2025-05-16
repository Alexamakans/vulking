#pragma once

#include "Common.hpp"
#include "Device.hpp"
#include <vulkan/vulkan_core.h>

namespace Vulking {
class Image {
public:
  MOVE_ONLY(Image);

  Image() = default;
  Image(const Device &device, VkImageCreateInfo info,
        VkMemoryPropertyFlags memoryProperties, const char *name = "unnamed") {
    init(device, info, memoryProperties, name);
  };
  Image(const Device &device, uint32_t width, uint32_t height,
        uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format,
        VkImageTiling tiling, VkImageUsageFlags usage,
        VkMemoryPropertyFlags memoryProperties, const char *name = "unnamed")
      : device(device) {
    VkImageCreateInfo info{};
    info.sType = STYPE(IMAGE_CREATE_INFO);
    info.imageType = VK_IMAGE_TYPE_2D;
    info.extent.width = width;
    info.extent.height = height;
    info.mipLevels = mipLevels;
    info.arrayLayers = 1;
    info.format = format;
    info.tiling = tiling;
    info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    info.usage = usage;
    info.samples = numSamples;
    info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    init(device, info, memoryProperties, name);
  };

  ~Image() {
    if (view != VK_NULL_HANDLE) {
      vkDestroyImageView(device, view, ALLOCATOR);
    }
    vkDestroyImage(device, image, ALLOCATOR);
    vkFreeMemory(device, memory, ALLOCATOR);
  }

  // TODO: Implement createImageView
  VkImageView createImageView();
  VkImageView view = VK_NULL_HANDLE;

private:
  void init(const Device &device, VkImageCreateInfo info,
            VkMemoryPropertyFlags memoryProperties, const char *name) {
    CHK(vkCreateImage(device, &info, ALLOCATOR, &image),
        std::format("failed to create {}_image", name));
    NAME_OBJECT(device, VK_OBJECT_TYPE_IMAGE, image, name);

    VkMemoryRequirements memoryRequirements;
    vkGetImageMemoryRequirements(device, image, &memoryRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = STYPE(MEMORY_ALLOCATE_INFO);
    allocInfo.allocationSize = memoryRequirements.size;
    allocInfo.memoryTypeIndex = device.getPhysical().findMemoryType(
        memoryRequirements.memoryTypeBits, memoryProperties);

    CHK(vkAllocateMemory(device, &allocInfo, ALLOCATOR, &memory),
        std::format("failed to allocate {}_memory", name));
    NAME_OBJECT(device, VK_OBJECT_TYPE_DEVICE_MEMORY, memory, name);

    CHK(vkBindImageMemory(device, image, memory, 0),
        std::format("failed to bind {}_memory to {}_image", name, name));
  }

  VkDevice device;

  VkImage image;
  VkDeviceMemory memory;
};
} // namespace Vulking
