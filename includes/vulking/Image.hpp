#pragma once

#include "Common.hpp"

namespace Vulking {
class Image {
public:
  Image(const Image &) = delete;
  Image &operator=(const Image &) = delete;
  Image(Image &&other) noexcept
      : image(std::move(other.image)), memory(std::move(other.memory)) {}
  Image &operator=(Image &&other) noexcept {
    if (this != &other) {
      image = std::move(other.image);
      memory = std::move(other.memory);
    }
    return *this;
  }

  Image() = default;
  Image(vk::ImageCreateInfo info, vk::MemoryPropertyFlags memoryProperties,
        const char *name = "unnamed");

  Image(const std::string &path, vk::SampleCountFlagBits samples,
        vk::Format format, const char *name = "unnamed");

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
