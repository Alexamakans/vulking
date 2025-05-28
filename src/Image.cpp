#include "Image.hpp"

#include "Buffer.hpp"
#include "Common.hpp"
#include "Engine.hpp"
#include "Functions.hpp"
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_structs.hpp>

namespace Vulking {
Image::Image(vk::ImageCreateInfo info, vk::MemoryPropertyFlags memoryProperties,
             const char *name) {
  init(info, memoryProperties, name);
};

Image::Image(uint32_t width, uint32_t height, uint32_t mipLevels,
             vk::SampleCountFlagBits samples, vk::Format format,
             vk::ImageTiling tiling, vk::ImageUsageFlags usage,
             vk::MemoryPropertyFlags memoryProperties, const char *name) {
  auto info = vk::ImageCreateInfo{}
                  .setImageType(vk::ImageType::e2D)
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

Image::Image(const std::string &path, vk::SampleCountFlagBits samples,
             vk::Format format, const char *name) {
  const auto [data, width, height] = loadRgba8888Texture(path.c_str());
  const auto mipLevels =
      1 + static_cast<uint32_t>(std::floor(std::log2(std::min(width, height))));

#define Usage(x) vk::ImageUsageFlagBits::x
  auto info =
      vk::ImageCreateInfo()
          .setImageType(vk::ImageType::e2D)
          .setExtent(vk::Extent3D(width, height))
          .setMipLevels(mipLevels)
          .setArrayLayers(1)
          .setFormat(format)
          .setTiling(vk::ImageTiling::eOptimal)
          .setInitialLayout(vk::ImageLayout::eUndefined)
          .setUsage(Usage(eTransferSrc) | Usage(eTransferDst) | Usage(eSampled))
          .setSamples(samples)
          .setSharingMode(vk::SharingMode::eExclusive);
  init(info, vk::MemoryPropertyFlagBits::eDeviceLocal, name);
#undef Usage
  Buffer<char> buffer(data, BufferUsage::STAGING, BufferMemory::STAGING);

#define Transition(from, to)                                                   \
  transitionImageLayout(image, format, vk::ImageLayout::from,                  \
                        vk::ImageLayout::to);
  copyBufferToImage(buffer.getBuffer(), image.get(), width, height);
#undef Transition
}

void Image::init(vk::ImageCreateInfo info,
                 vk::MemoryPropertyFlags memoryProperties, const char *name) {
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
} // namespace Vulking
