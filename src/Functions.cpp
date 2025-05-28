#include "Functions.hpp"

#include "Common.hpp"
#include "Engine.hpp"
#include <vulkan/vulkan_handles.hpp>

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

  const auto info = vk::SamplerCreateInfo{}
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
                           uint32_t mipLevels, vk::ImageLayout from,
                           vk::ImageLayout to) {
  const auto cmd = Engine::beginCommand();
  {
    auto barrier = vk::ImageMemoryBarrier2KHR()
                       .setOldLayout(from)
                       .setNewLayout(to)
                       .setSrcQueueFamilyIndex(vk::QueueFamilyIgnored)
                       .setDstQueueFamilyIndex(vk::QueueFamilyIgnored)
                       .setImage(image)
                       .setSubresourceRange(
                           vk::ImageSubresourceRange()
                               .setAspectMask(vk::ImageAspectFlagBits::eColor)
                               .setLevelCount(mipLevels)
                               .setLayerCount(1));

    if (from == vk::ImageLayout::eUndefined &&
        to == vk::ImageLayout::eTransferDstOptimal) {
      barrier.setSrcAccessMask(vk::AccessFlagBits2::eNone)
          .setDstAccessMask(vk::AccessFlagBits2::eTransferWrite)
          .setSrcStageMask(vk::PipelineStageFlagBits2::eTopOfPipe)
          .setDstStageMask(vk::PipelineStageFlagBits2::eTransfer);
    } else if (from == vk::ImageLayout::eTransferDstOptimal &&
               to == vk::ImageLayout::eShaderReadOnlyOptimal) {
      barrier.setSrcAccessMask(vk::AccessFlagBits2::eTransferWrite)
          .setDstAccessMask(vk::AccessFlagBits2::eShaderRead)
          .setSrcStageMask(vk::PipelineStageFlagBits2::eTransfer)
          .setDstStageMask(vk::PipelineStageFlagBits2::eFragmentShader);
    } else {
      throw std::invalid_argument("unsupported layout transition");
    }

    auto dependencyInfo =
        vk::DependencyInfoKHR().setImageMemoryBarriers({barrier});
    cmd.pipelineBarrier2(dependencyInfo);
  }
  cmd.end();
}

void copyBufferToImage(vk::Buffer buffer, vk::Image image, uint32_t width,
                       uint32_t height) {
  const auto cmd = Engine::beginCommand();
  {
    const auto region =
        vk::BufferImageCopy()
            .setImageSubresource(
                vk::ImageSubresourceLayers()
                    .setAspectMask(vk::ImageAspectFlagBits::eColor)
                    .setLayerCount(0))
            .setImageExtent({width, height, 1});

    cmd.copyBufferToImage(buffer, image, vk::ImageLayout::eTransferDstOptimal,
                          region);
  }
  cmd.end();
}

void generateMipmaps(const vk::Image image, const vk::Format format,
                     const int32_t width, const int32_t height,
                     const uint32_t mipLevels) {
  const auto formatProperties =
      Engine::physicalDevice.getFormatProperties(format);

  if (!(formatProperties.optimalTilingFeatures &
        vk::FormatFeatureFlagBits::eSampledImageFilterLinear)) {
    throw std::runtime_error("image format does not support linear blitting");
  }

  const auto cmd = Engine::beginCommand();
  {
    auto barrier = vk::ImageMemoryBarrier2KHR()
                       .setImage(image)
                       .setSrcQueueFamilyIndex(vk::QueueFamilyIgnored)
                       .setDstQueueFamilyIndex(vk::QueueFamilyIgnored)
                       .setSubresourceRange(
                           vk::ImageSubresourceRange()
                               .setAspectMask(vk::ImageAspectFlagBits::eColor)
                               .setLayerCount(1)
                               .setLevelCount(1));

    int32_t mipWidth = width;
    int32_t mipHeight = height;

    vk::DependencyInfoKHR dependencyInfo{};

    for (uint32_t i = 1; i < mipLevels; i++) {
      barrier.setOldLayout(vk::ImageLayout::eTransferDstOptimal)
          .setNewLayout(vk::ImageLayout::eTransferSrcOptimal)
          .setSrcAccessMask(vk::AccessFlagBits2::eTransferWrite)
          .setDstAccessMask(vk::AccessFlagBits2::eTransferRead)
          .setSrcStageMask(vk::PipelineStageFlagBits2::eTransfer)
          .setDstStageMask(vk::PipelineStageFlagBits2::eTransfer)
          .subresourceRange.setBaseMipLevel(i - 1);

      cmd.pipelineBarrier2KHR(dependencyInfo.setImageMemoryBarriers({barrier}));

      auto blitRegion =
          vk::ImageBlit2KHR()
              .setSrcOffsets({
                  vk::Offset3D{0, 0, 0},
                  vk::Offset3D{mipWidth, mipHeight, 1},
              })
              .setSrcSubresource(
                  vk::ImageSubresourceLayers()
                      .setAspectMask(vk::ImageAspectFlagBits::eColor)
                      .setMipLevel(i - 1)
                      .setLayerCount(1))
              .setDstOffsets(
                  {vk::Offset3D{0, 0, 0},
                   vk::Offset3D{mipWidth > 1 ? mipWidth / 2 : 1,
                                mipHeight > 1 ? mipHeight / 2 : 1, 1}})
              .setDstSubresource(
                  vk::ImageSubresourceLayers()
                      .setAspectMask(vk::ImageAspectFlagBits::eColor)
                      .setMipLevel(i)
                      .setLayerCount(1));
      auto blitInfo =
          vk::BlitImageInfo2KHR()
              .setSrcImage(image)
              .setSrcImageLayout(vk::ImageLayout::eTransferSrcOptimal)
              .setDstImage(image)
              .setDstImageLayout(vk::ImageLayout::eTransferDstOptimal)
              .setFilter(vk::Filter::eLinear)
              .setRegions({blitRegion});

      cmd.blitImage2KHR(blitInfo);

      barrier.setOldLayout(vk::ImageLayout::eTransferSrcOptimal)
          .setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
          .setSrcAccessMask(vk::AccessFlagBits2::eTransferRead)
          .setDstAccessMask(vk::AccessFlagBits2::eShaderRead)
          .setSrcStageMask(vk::PipelineStageFlagBits2::eTransfer)
          .setDstStageMask(vk::PipelineStageFlagBits2::eFragmentShader);

      cmd.pipelineBarrier2KHR(dependencyInfo.setImageMemoryBarriers({barrier}));

      if (mipWidth > 1) {
        mipWidth /= 2;
      }
      if (mipHeight > 1) {
        mipHeight /= 2;
      }
    }

    barrier.setOldLayout(vk::ImageLayout::eTransferDstOptimal)
        .setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
        .setSrcAccessMask(vk::AccessFlagBits2::eTransferWrite)
        .setDstAccessMask(vk::AccessFlagBits2::eShaderRead)
        .setSrcStageMask(vk::PipelineStageFlagBits2::eTransfer)
        .setDstStageMask(vk::PipelineStageFlagBits2::eFragmentShader)
        .subresourceRange.setBaseMipLevel(mipLevels - 1);

    cmd.pipelineBarrier2KHR(dependencyInfo.setImageMemoryBarriers({barrier}));
  }
  cmd.end();

  I JUST FINISHED THIS FUNCTION
}
} // namespace Vulking
