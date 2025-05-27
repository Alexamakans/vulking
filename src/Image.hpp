#pragma once

#include "Common.hpp"

namespace Vulking {
class Image {
public:
  MOVE_ONLY(Image);

  Image() = default;
  Image(vk::ImageCreateInfo info, vk::MemoryPropertyFlags memoryProperties,
        const char *name = "unnamed");
  Image(uint32_t width, uint32_t height, uint32_t mipLevels,
        vk::SampleCountFlagBits samples, vk::Format format,
        vk::ImageTiling tiling, vk::ImageUsageFlags usage,
        vk::MemoryPropertyFlags memoryProperties, const char *name = "unnamed");

  vk::UniqueImage image;
  vk::UniqueDeviceMemory memory;

private:
  void init(vk::ImageCreateInfo info, vk::MemoryPropertyFlags memoryProperties,
            const char *name);
};
} // namespace Vulking
