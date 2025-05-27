#include "Functions.hpp"

#include "Engine.hpp"

vk::Format
Vulking::findSupportedFormat(const std::vector<vk::Format> &candidates,
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

vk::Format Vulking::findDepthFormat() {
  return findSupportedFormat(
      {vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint,
       vk::Format::eD24UnormS8Uint},
      vk::ImageTiling::eOptimal,
      vk::FormatFeatureFlagBits::eDepthStencilAttachment);
}

vk::AttachmentDescription
Vulking::ColorAttachmentDescription(vk::Format format,
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
Vulking::DepthAttachmentDescription(vk::SampleCountFlagBits msaaSamples) {
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

vk::AttachmentDescription
Vulking::ColorResolveAttachmentDescription(vk::Format format) {
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

vk::RenderPassCreateInfo Vulking::RenderPassCreateInfo() {
  auto colorAttachmentDescription = ColorAttachmentDescription(
      Engine::swapchainImageFormat, Engine::msaaSamples);
  auto depthAttachmentDescription =
      DepthAttachmentDescription(Engine::msaaSamples);
  auto colorResolveAttachmentDescription =
      ColorResolveAttachmentDescription(Engine::swapchainImageFormat);

  std::array<vk::AttachmentDescription, 3> attachments{
      colorAttachmentDescription,
      depthAttachmentDescription,
      colorResolveAttachmentDescription,
  };

  auto colorAttachmentRef =
      vk::AttachmentReference{}.setAttachment(0).setLayout(
          vk::ImageLayout::eColorAttachmentOptimal);
  auto depthAttachmentRef =
      vk::AttachmentReference{}.setAttachment(1).setLayout(
          vk::ImageLayout::eDepthStencilAttachmentOptimal);
  auto colorResolveAttachmentRef =
      vk::AttachmentReference{}.setAttachment(2).setLayout(
          vk::ImageLayout::eColorAttachmentOptimal);

  std::array<vk::SubpassDescription, 1> subpasses{
      vk::SubpassDescription{}
          .setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
          .setColorAttachments({colorAttachmentRef})
          .setPDepthStencilAttachment(&depthAttachmentRef)
          .setResolveAttachments({colorResolveAttachmentRef})};

  auto stageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput |
                   vk::PipelineStageFlagBits::eEarlyFragmentTests;
  std::array<vk::SubpassDependency, 1> dependencies{
      vk::SubpassDependency{}
          .setSrcSubpass(vk::SubpassExternal)
          .setDstSubpass(0)
          .setSrcStageMask(stageMask)
          //.setSrcAccessMask(0)
          .setDstStageMask(stageMask)
          .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite |
                            vk::AccessFlagBits::eDepthStencilAttachmentWrite)};

  return vk::RenderPassCreateInfo{}
      .setAttachments(attachments)
      .setSubpasses(subpasses)
      .setDependencies(dependencies);
}
