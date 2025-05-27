#include "Functions.hpp"

#include "Engine.hpp"
#include <iterator>
#include <vulkan/vulkan.hpp>

namespace Vulking {
vk::Format findSupportedFormat(const std::vector<vk::Format> &candidates,
                               vk::ImageTiling tiling,
                               vk::FormatFeatureFlagBits features) {
  for (vk::Format format : candidates) {
    auto props = Engine::physicalDevice.getFormatProperties(format);

    if (tiling == vk::ImageTiling::eLinear &&
        (props.linearTilingFeatures & features) == features) {
      return format;
    } else if (tiling == vk::ImageTiling::eOptimal &&
               (props.optimalTilingFeatures & features) == features) {
      return format;
    }
  }

  throw std::runtime_error("failed to find supported format!");
}

vk::Format findDepthFormat() {
  return findSupportedFormat(
      {vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint,
       vk::Format::eD24UnormS8Uint},
      vk::ImageTiling::eOptimal,
      vk::FormatFeatureFlagBits::eDepthStencilAttachment);
}

vk::AttachmentDescription
ColorAttachmentDescription(vk::Format format,
                           vk::SampleCountFlagBits msaaSamples) {
  return vk::AttachmentDescription{}
      .setFormat(format)
      .setSamples(msaaSamples)
      .setLoadOp(vk::AttachmentLoadOp::eClear)
      .setStoreOp(vk::AttachmentStoreOp::eStore)
      .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
      .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
      .setInitialLayout(vk::ImageLayout::eUndefined)
      .setFinalLayout(vk::ImageLayout::eColorAttachmentOptimal);
}

vk::AttachmentDescription
DepthAttachmentDescription(vk::SampleCountFlagBits msaaSamples) {
  return vk::AttachmentDescription{}
      .setFormat(findDepthFormat())
      .setSamples(msaaSamples)
      .setLoadOp(vk::AttachmentLoadOp::eClear)
      .setStoreOp(vk::AttachmentStoreOp::eStore)
      .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
      .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
      .setInitialLayout(vk::ImageLayout::eUndefined)
      .setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);
}

vk::AttachmentDescription ColorResolveAttachmentDescription(vk::Format format) {
  return vk::AttachmentDescription{}
      .setFormat(format)
      .setSamples(vk::SampleCountFlagBits::e1)
      .setLoadOp(vk::AttachmentLoadOp::eDontCare)
      .setStoreOp(vk::AttachmentStoreOp::eStore)
      .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
      .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
      .setInitialLayout(vk::ImageLayout::eUndefined)
      .setFinalLayout(vk::ImageLayout::ePresentSrcKHR);
}

struct RenderPassCreateInfoComponents {
  std::array<vk::AttachmentDescription, 3> attachments;
  std::array<vk::SubpassDescription, 1> subpasses;
  std::array<vk::SubpassDependency, 1> dependencies;

  vk::RenderPassCreateInfo toCreateInfo() const {
    return vk::RenderPassCreateInfo{}
        .setAttachments(attachments)
        .setSubpasses(subpasses)
        .setDependencies(dependencies);
  }
};

vk::UniqueDescriptorPool createDescriptorPool(
    uint32_t size,
    const std::vector<std::tuple<vk::DescriptorType, uint32_t>> &poolSizes) {
  auto _poolSizes = std::vector<vk::DescriptorPoolSize>(poolSizes.size());
  for (size_t i = 0; i < poolSizes.size(); i++) {
    auto [type, size] = poolSizes[i];
    _poolSizes.push_back(
        vk::DescriptorPoolSize{}.setType(type).setDescriptorCount(size));
  }
  auto info = vk::DescriptorPoolCreateInfo{}
                  .setPoolSizeCount(1)
                  .setPoolSizes(_poolSizes)
                  .setMaxSets(size);
  return Engine::device->createDescriptorPoolUnique(info);
}

std::vector<vk::UniqueDescriptorSet>
allocateDescriptorSet(const vk::UniqueDescriptorPool &pool,
                      const std::vector<vk::DescriptorSetLayout> &layouts) {
  auto info = vk::DescriptorSetAllocateInfo{}
                  .setDescriptorPool(pool.get())
                  .setSetLayouts(layouts);
  return Engine::device->allocateDescriptorSetsUnique(info);
}

vk::UniqueSampler createSampler() {
  const auto properties = Vulking::Engine::physicalDevice.getProperties();
  const auto maxSamplerAnisotropy = properties.limits.maxSamplerAnisotropy;

  auto info = vk::SamplerCreateInfo{}
                  .setMagFilter(vk::Filter::eLinear)
                  .setMinFilter(vk::Filter::eLinear)
                  .setAddressModeU(vk::SamplerAddressMode::eRepeat)
                  .setAddressModeV(vk::SamplerAddressMode::eRepeat)
                  .setAddressModeW(vk::SamplerAddressMode::eRepeat)
                  .setAnisotropyEnable(vk::True)
                  .setMaxAnisotropy(maxSamplerAnisotropy)
                  .setBorderColor(vk::BorderColor::eIntOpaqueBlack)
                  .setUnnormalizedCoordinates(vk::False)
                  .setCompareEnable(vk::False)
                  .setCompareOp(vk::CompareOp::eAlways)
                  .setMipmapMode(vk::SamplerMipmapMode::eLinear)
                  .setMinLod(0.0f)
                  .setMaxLod(vk::LodClampNone)
                  .setMipLodBias(0.0f);
  return Vulking::Engine::device->createSamplerUnique(info);
}

void transitionImageLayout(vk::Image image, vk::Format format,
                           vk::ImageLayout from, vk::ImageLayout to) {}

void copyBufferToImage(vk::Buffer buffer, vk::Image image, uint32_t width,
                       uint32_t height) {}

void generateMipmaps(vk::Image image, vk::Format format, int32_t width,
                     int32_t height, uint32_t mipLevels) {}
} // namespace Vulking
