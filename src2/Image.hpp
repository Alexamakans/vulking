#pragma once

#include "Common.hpp"
#include "Engine.hpp"

namespace Vulking {
class Image {
public:
  MOVE_ONLY(Image);

  Image() = default;
  Image(vk::ImageCreateInfo info, vk::MemoryPropertyFlags memoryProperties,
        const char *name = "unnamed") {
    init(info, memoryProperties, name);
  };
  Image(uint32_t width, uint32_t height, uint32_t mipLevels,
        vk::SampleCountFlagBits samples, vk::Format format,
        vk::ImageTiling tiling, vk::ImageUsageFlags usage,
        vk::MemoryPropertyFlags memoryProperties,
        const char *name = "unnamed") {
    vk::ImageCreateInfo info;
    info.setImageType(vk::ImageType::e2D)
        .setExtent(vk::Extent3D(width, height))
        .setMipLevels(mipLevels)
        .setArrayLayers(1)
        .setFormat(format)
        .setTiling(tiling)
        .setInitialLayout(vk::ImageLayout::eUndefined)
        .setUsage(usage)
        .setSamples(samples)
        .setSharingMode(vk::SharingMode::eExclusive);

    init(info, memoryProperties, name);
  };

  ~Image() {
    if (!view) {
      Engine::device->destroyImageView(view, ALLOCATOR);
    }
    Engine::device->destroyImage(image, ALLOCATOR);
    Engine::device->freeMemory(memory, ALLOCATOR);
  }

  // TODO: Implement createImageView
  vk::ImageView createImageView();
  vk::ImageView view = VK_NULL_HANDLE;

private:
  void init(vk::ImageCreateInfo info, vk::MemoryPropertyFlags memoryProperties,
            const char *name) {
    image = Engine::device->createImage(info, ALLOCATOR);
    NAME_OBJECT(Engine::device, image, name);

    auto memoryRequirements = Engine::device->getImageMemoryRequirements(image);

    vk::MemoryAllocateInfo allocInfo;
    allocInfo.setAllocationSize(memoryRequirements.size);
    allocInfo.setMemoryTypeIndex(
        findMemoryType(Engine::physicalDevice,
                       memoryRequirements.memoryTypeBits, memoryProperties));

    memory = Engine::device->allocateMemory(allocInfo, ALLOCATOR);
    NAME_OBJECT(Engine::device, memory, name);

    Engine::device->bindImageMemory(image, memory, 0);
  }

  vk::Image image;
  vk::DeviceMemory memory;
};
} // namespace Vulking
