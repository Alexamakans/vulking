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

vk::RenderPassCreateInfo RenderPassCreateInfo();
} // namespace Vulking
