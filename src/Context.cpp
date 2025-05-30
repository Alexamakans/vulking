#include "Context.hpp"
#include "Engine.hpp"

namespace Vulking {
void Swapchain::createFramebuffers(const vk::RenderPass &renderPass) {
  const auto device = Engine::ctx().device.get();
  framebuffers.resize(images.size());
  for (uint32_t i = 0; i < images.size(); i++) {
    std::array<vk::ImageView, 3> attachments = {
        colorView.get(),
        depthView.get(),
        views[i].get(),
    };

    auto info = vk::FramebufferCreateInfo{}
                    .setRenderPass(renderPass)
                    .setAttachments(attachments)
                    .setWidth(extent.width)
                    .setHeight(extent.height)
                    .setLayers(1);

    framebuffers[i] = device.createFramebufferUnique(info);
  }
}

vk::Framebuffer Swapchain::getFramebuffer() {
  return framebuffers[getCurrentResourceIndex()].get();
}

uint32_t Swapchain::getCurrentResourceIndex() {
  return Engine::ctx().frame % imageCount;
}

vk::CommandBuffer Context::beginCommand() {
  vk::CommandBufferAllocateInfo info;
  info.setCommandPool(commandPool.get());
  info.setLevel(vk::CommandBufferLevel::ePrimary);
  info.setCommandBufferCount(1);
  auto commandBuffer = device->allocateCommandBuffers(info).front();

  vk::CommandBufferBeginInfo beginInfo;
  beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
  commandBuffer.begin(beginInfo);
  return commandBuffer;
}

void Context::endAndSubmitGraphicsCommand(vk::CommandBuffer &&cmd) {
  cmd.end();
  graphicsQueue.submit(vk::SubmitInfo().setCommandBuffers(cmd));
  device->waitIdle();
  device->freeCommandBuffers(commandPool.get(), cmd);
}

std::optional<std::tuple<vk::CommandBuffer, uint32_t>> Context::beginRender() {
  const auto index = swapchain.getCurrentResourceIndex();
  const auto waitFenceResult =
      device->waitForFences(inFlightFences[index].get(), vk::True, UINT64_MAX);
  if (waitFenceResult == vk::Result::eErrorDeviceLost) {
    throw std::runtime_error("device lost");
  }

  const auto acquire =
      device->acquireNextImageKHR(swapchain.handle.get(), UINT64_MAX,
                                  imageAvailableSemaphores[index].get());
  const auto acquireResult = acquire.result;

  if (acquireResult == vk::Result::eErrorOutOfDateKHR) {
    // recreate swapchain
    throw std::runtime_error("swapchain recreation not implemented yet");
    return std::nullopt;
  } else if (acquireResult != vk::Result::eSuccess &&
             acquireResult != vk::Result::eSuboptimalKHR) {
    throw std::runtime_error("failed to acquire swapchain image");
  }

  swapchain.currentImageIndex = acquire.value;
  device->resetFences(inFlightFences[index].get());
  commandBuffers[index]->reset();

  return std::make_tuple(commandBuffers[index].get(),
                         swapchain.currentImageIndex);
}

void Context::endRender(const std::vector<vk::CommandBuffer> &commandBuffers) {
  const auto index = swapchain.getCurrentResourceIndex();

  std::vector<vk::PipelineStageFlags> waitDstStageMask{
      vk::PipelineStageFlagBits::eColorAttachmentOutput};
  const auto submitInfo =
      vk::SubmitInfo()
          .setWaitSemaphores({imageAvailableSemaphores[index].get()})
          .setWaitDstStageMask(waitDstStageMask)
          .setCommandBuffers(commandBuffers)
          .setSignalSemaphores({renderFinishedSemaphores[index].get()});

  graphicsQueue.submit(submitInfo, inFlightFences[index].get());

  const auto presentInfo =
      vk::PresentInfoKHR{}
          .setWaitSemaphores({renderFinishedSemaphores[index].get()})
          .setSwapchains({swapchain.handle.get()})
          .setImageIndices({swapchain.currentImageIndex});

  const auto presentResult = presentQueue.presentKHR(presentInfo);
  if (presentResult == vk::Result::eErrorOutOfDateKHR ||
      presentResult == vk::Result::eSuboptimalKHR ||
      swapchain.framebufferResized) {
    swapchain.framebufferResized = false;
    // recreate swapchain
    throw std::runtime_error("swapchain recreation not implemented yet");
  } else if (presentResult != vk::Result::eSuccess) {
    throw std::runtime_error("failed to present swapchain image");
  }

  ++frame;
}

vk::ImageView Context::createImageView(vk::Image image, vk::Format format,
                                       vk::ImageAspectFlags aspectFlags,
                                       uint32_t mipLevels, const char *name) {
  vk::ImageViewCreateInfo info{};
  info.setImage(image)
      .setViewType(vk::ImageViewType::e2D)
      .setFormat(format)
      .setSubresourceRange(vk::ImageSubresourceRange{}
                               .setAspectMask(aspectFlags)
                               .setBaseMipLevel(0)
                               .setLevelCount(mipLevels)
                               .setBaseArrayLayer(0)
                               .setLayerCount(1));

  auto obj = device->createImageView(info);
  NAME_OBJECT(device, obj, name);
  return obj;
}

vk::UniqueImageView
Context::createImageViewUnique(vk::Image image, vk::Format format,
                               vk::ImageAspectFlags aspectFlags,
                               uint32_t mipLevels, const char *name) {
  vk::ImageViewCreateInfo info{};
  info.setImage(image)
      .setViewType(vk::ImageViewType::e2D)
      .setFormat(format)
      .setSubresourceRange(vk::ImageSubresourceRange{}
                               .setAspectMask(aspectFlags)
                               .setBaseMipLevel(0)
                               .setLevelCount(mipLevels)
                               .setBaseArrayLayer(0)
                               .setLayerCount(1));

  auto obj = device->createImageViewUnique(info);
  NAME_OBJECT(device, obj.get(), name);
  return std::move(obj);
}

} // namespace Vulking
