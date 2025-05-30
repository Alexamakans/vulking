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
                  .setExtent(vk::Extent3D(width, height, 1))
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
  // loading and mipmapping should be separated into their own functions or
  // something loading should probably go in Common.hpp or Util.hpp
  const auto [data, width, height] = loadRgba8888Texture(path.c_str());
  const auto mipLevels =
      1 + static_cast<uint32_t>(std::floor(std::log2(std::min(width, height))));

#define Usage(x) vk::ImageUsageFlagBits::x
  auto info =
      vk::ImageCreateInfo()
          .setImageType(vk::ImageType::e2D)
          .setExtent(vk::Extent3D(width, height, 1))
          .setMipLevels(mipLevels)
          .setArrayLayers(1)
          .setFormat(format)
          .setTiling(vk::ImageTiling::eOptimal)
          .setInitialLayout(vk::ImageLayout::eUndefined)
          .setUsage(Usage(eTransferSrc) | Usage(eTransferDst) | Usage(eSampled))
          .setSamples(vk::SampleCountFlagBits::e1)
          .setSharingMode(vk::SharingMode::eExclusive);
  init(info, vk::MemoryPropertyFlagBits::eDeviceLocal, name);
#undef Usage
  Buffer<char> buffer(data, BufferUsage::STAGING, BufferMemory::STAGING);

  transitionImageLayout(image.get(), format, mipLevels,
                        vk::ImageLayout::eUndefined,
                        vk::ImageLayout::eTransferDstOptimal);
  copyBufferToImage(buffer.getBuffer(), image.get(), width, height);
  // transitioned to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL while generating
  // mipmaps
  generateMipmaps(image.get(), format, static_cast<int32_t>(width),
                  static_cast<int32_t>(height), mipLevels);
}

void Image::init(vk::ImageCreateInfo info,
                 vk::MemoryPropertyFlags memoryProperties, const char *name) {
  std::cout << "Device: " << std::hex << Engine::ctx().device.get()
            << std::endl;
  image = Engine::ctx().device->createImageUnique(info);
  NAME_OBJECT(Engine::ctx().device, image.get(), name);

  auto memoryRequirements =
      Engine::ctx().device->getImageMemoryRequirements(image.get());

  vk::MemoryAllocateInfo allocInfo;
  allocInfo.setAllocationSize(memoryRequirements.size);
  allocInfo.setMemoryTypeIndex(findMemoryType(Engine::ctx().physicalDevice,
                                              memoryRequirements.memoryTypeBits,
                                              memoryProperties));

  memory = Engine::ctx().device->allocateMemoryUnique(allocInfo);
  NAME_OBJECT(Engine::ctx().device, memory.get(), name);

  Engine::ctx().device->bindImageMemory(image.get(), memory.get(), 0);
  mipLevels = info.mipLevels;
  width = info.extent.width;
  height = info.extent.height;
}
} // namespace Vulking
