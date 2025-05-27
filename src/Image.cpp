#include "Image.hpp"

#include "Engine.hpp"

Vulking::Image::Image(vk::ImageCreateInfo info,
                      vk::MemoryPropertyFlags memoryProperties,
                      const char *name) {
  init(info, memoryProperties, name);
};

Vulking::Image::Image(uint32_t width, uint32_t height, uint32_t mipLevels,
                      vk::SampleCountFlagBits samples, vk::Format format,
                      vk::ImageTiling tiling, vk::ImageUsageFlags usage,
                      vk::MemoryPropertyFlags memoryProperties,
                      const char *name) {
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

Vulking::Image::Image(const std::string &path, vk::SampleCountFlagBits samples,
                      vk::Format format, const char *name) {

MAYBE CONTINUE HERE?
}

void Vulking::Image::init(vk::ImageCreateInfo info,
                          vk::MemoryPropertyFlags memoryProperties,
                          const char *name) {
  image = Engine::device->createImageUnique(info);
  NAME_OBJECT(Engine::device, image.get(), name);

  auto memoryRequirements =
      Engine::device->getImageMemoryRequirements(image.get());

  vk::MemoryAllocateInfo allocInfo;
  allocInfo.setAllocationSize(memoryRequirements.size);
  allocInfo.setMemoryTypeIndex(findMemoryType(Engine::physicalDevice,
                                              memoryRequirements.memoryTypeBits,
                                              memoryProperties));

  memory = Engine::device->allocateMemoryUnique(allocInfo);
  NAME_OBJECT(Engine::device, memory.get(), name);

  Engine::device->bindImageMemory(image.get(), memory.get(), 0);
}
