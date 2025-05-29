#pragma once

#include "Common.hpp"

namespace Vulking {
class Image {
public:
  Image(const Image &) = delete;
  Image &operator=(const Image &) = delete;
  Image(Image &&other) noexcept
      : image(std::move(other.image)), memory(std::move(other.memory)),
        mipLevels(other.mipLevels), width(other.width), height(other.height) {}
  Image &operator=(Image &&other) noexcept {
    if (this != &other) {
      image = std::move(other.image);
      memory = std::move(other.memory);
      mipLevels = other.mipLevels;
      width = other.width;
      height = other.height;
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

  uint32_t getMipLevels() const { return mipLevels; }
  uint32_t getWidth() const { return width; }
  uint32_t getHeight() const { return height; }

  vk::UniqueImage image;
  vk::UniqueDeviceMemory memory;

private:
  void init(vk::ImageCreateInfo info, vk::MemoryPropertyFlags memoryProperties,
            const char *name);

  uint32_t mipLevels = 1;
  uint32_t width, height;
};
} // namespace Vulking
