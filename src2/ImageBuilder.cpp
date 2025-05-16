#include "ImageBuilder.hpp"

namespace Vulking {
ImageBuilder &ImageBuilder::size(uint32_t width, uint32_t height) {
  assert(width != 0);
  assert(height != 0);
  info.extent.width = width;
  info.extent.height = height;
  return *this;
}

ImageBuilder ImageBuilder::setMipLevels(uint32_t mipLevels) {
  info.mipLevels = mipLevels;
  return *this;
}

ImageBuilder ImageBuilder::calculateMipLevels() {
  assert(info.extent.width != 0);
  assert(info.extent.height != 0);
  info.mipLevels = 1 + static_cast<uint32_t>(std::floor(std::log2(
                           std::max(info.extent.width, info.extent.height))));
  return *this;
}

ImageBuilder &ImageBuilder::usage(VkImageUsageFlags usage) {
  info.usage = usage;
  return *this;
}

ImageBuilder &ImageBuilder::memoryProperties(VkMemoryPropertyFlags properties) {
  properties = properties;
  return *this;
}

ImageBuilder &ImageBuilder::multisamples(const Device &device) {
  info.samples = device.getPhysical().getMsaaSamples();
  return *this;
}

ImageBuilder &ImageBuilder::multisamples(VkSampleCountFlagBits samples) {
  info.samples = samples;
  return *this;
}

ImageBuilder &ImageBuilder::format(VkFormat format) {
  info.format = format;
  return *this;
}

ImageBuilder &ImageBuilder::flags(VkImageCreateFlags flags) {
  info.flags = flags;
  return *this;
};

ImageBuilder &ImageBuilder::imageType(VkImageType imageType) {
  info.imageType = imageType;
  return *this;
};

ImageBuilder &ImageBuilder::arrayLayers(uint32_t arrayLayers) {
  info.arrayLayers = arrayLayers;
  return *this;
};

ImageBuilder &ImageBuilder::tiling(VkImageTiling tiling) {
  info.tiling = tiling;
  return *this;
};

ImageBuilder &ImageBuilder::sharingMode(VkSharingMode sharingMode) {
  info.sharingMode = sharingMode;
  return *this;
};

ImageBuilder &
ImageBuilder::queueFamilyIndexCount(uint32_t queueFamilyIndexCount) {
  info.queueFamilyIndexCount = queueFamilyIndexCount;
  return *this;
};

ImageBuilder &
ImageBuilder::pQueueFamilyIndices(const uint32_t *pQueueFamilyIndices) {
  info.pQueueFamilyIndices = pQueueFamilyIndices;
  return *this;
};

ImageBuilder &ImageBuilder::initialLayout(VkImageLayout initialLayout) {
  info.initialLayout = initialLayout;
  return *this;
};

Image ImageBuilder::create(const Device &device, const char *name) {
  return Image(device, info, properties, name);
};
} // namespace Vulking
