#pragma once

#include "Common.hpp"

namespace Vulking {
vk::Format findSupportedFormat(const std::vector<vk::Format> &candidates,
                               vk::ImageTiling tiling,
                               vk::FormatFeatureFlagBits features);

vk::Format findDepthFormat();

vk::AttachmentDescription
ColorAttachmentDescription(vk::Format format,
                           vk::SampleCountFlagBits msaaSamples);

vk::AttachmentDescription
DepthAttachmentDescription(vk::SampleCountFlagBits msaaSamples);

vk::AttachmentDescription ColorResolveAttachmentDescription(vk::Format format);

struct RenderPassInfo {
  std::array<vk::AttachmentDescription, 3> attachments;

  // References needed by SubpassDescription
  vk::AttachmentReference colorAttachmentRef;
  vk::AttachmentReference depthAttachmentRef;
  vk::AttachmentReference colorResolveAttachmentRef;

  std::array<vk::SubpassDescription, 1> subpasses;
  std::array<vk::SubpassDependency, 1> dependencies;

  static RenderPassInfo Create(vk::Format colorFormat,
                               vk::SampleCountFlagBits msaaSamples) {
    RenderPassInfo info;

    info.attachments = {
        Vulking::ColorAttachmentDescription(colorFormat, msaaSamples),
        Vulking::DepthAttachmentDescription(msaaSamples),
        Vulking::ColorResolveAttachmentDescription(colorFormat),
    };

    info.colorAttachmentRef =
        vk::AttachmentReference{}.setAttachment(0).setLayout(
            vk::ImageLayout::eColorAttachmentOptimal);
    info.depthAttachmentRef =
        vk::AttachmentReference{}.setAttachment(1).setLayout(
            vk::ImageLayout::eDepthStencilAttachmentOptimal);
    info.colorResolveAttachmentRef =
        vk::AttachmentReference{}.setAttachment(2).setLayout(
            vk::ImageLayout::eColorAttachmentOptimal);

    info.subpasses = {
        vk::SubpassDescription{}
            .setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
            .setColorAttachments({info.colorAttachmentRef})
            .setPDepthStencilAttachment(&info.depthAttachmentRef)
            .setResolveAttachments({info.colorResolveAttachmentRef}),
    };

    auto stageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput |
                     vk::PipelineStageFlagBits::eEarlyFragmentTests;
    info.dependencies = {
        vk::SubpassDependency{}
            .setSrcSubpass(vk::SubpassExternal)
            .setDstSubpass(0)
            .setSrcStageMask(stageMask)
            .setDstStageMask(stageMask)
            .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite |
                              vk::AccessFlagBits::eDepthStencilAttachmentWrite),
    };

    return info;
  }

  vk::RenderPassCreateInfo toCreateInfo() const {
    return vk::RenderPassCreateInfo{}
        .setAttachments(attachments)
        .setSubpasses(subpasses)
        .setDependencies(dependencies);
  }
};

vk::UniqueDescriptorPool createDescriptorPool(
    uint32_t size,
    const std::vector<std::tuple<vk::DescriptorType, uint32_t>> &poolSizes);

std::vector<vk::UniqueDescriptorSet>
allocateDescriptorSet(const vk::UniqueDescriptorPool &pool,
                      const std::vector<vk::DescriptorSetLayout> &layouts);

vk::UniqueSampler createSampler();

void transitionImageLayout(vk::Image image, vk::Format format,
                           vk::ImageLayout from, vk::ImageLayout to);

void copyBufferToImage(vk::Buffer buffer, vk::Image image, uint32_t width,
                       uint32_t height);

void generateMipmaps(vk::Image image, vk::Format format, int32_t width,
                     int32_t height, uint32_t mipLevels);

} // namespace Vulking
