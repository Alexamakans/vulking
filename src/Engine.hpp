#pragma once

#include "Common.hpp"
#include "Image.hpp"
#include "ScopedSurface.hpp"
#include <limits>
#include <optional>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_handles.hpp>

#define VULKING_MAKE_VERSION(major, minor, patch)                              \
  VK_MAKE_VERSION(major, minor, patch)

namespace Vulking {
class Engine {
public:
  Engine(GLFWwindow *window, const char *applicationInfo,
         uint32_t applicationVersion,
         const std::vector<const char *> &requiredExtensions);

  static vk::CommandBuffer beginCommand();

  /* tuple<command buffer to populate, current swapchain resourceIndex> */
  std::optional<std::tuple<vk::CommandBuffer, uint32_t>> beginRender() {
    const auto index = getCurrentSwapchainResourceIndex();
    const auto waitFenceResult =
        device->waitForFences(inFlightFences[index], vk::True, UINT64_MAX);
    if (waitFenceResult == vk::Result::eErrorDeviceLost) {
      throw std::runtime_error("device lost");
    }

    const auto acquire = device->acquireNextImageKHR(
        Engine::swapchain.get(), UINT64_MAX, imageAvailableSemaphores[index]);
    const auto acquireResult = acquire.result;

    if (acquireResult == vk::Result::eErrorOutOfDateKHR) {
      // recreate swapchain
      throw std::runtime_error("swapchain recreation not implemented yet");
      return std::nullopt;
    } else if (acquireResult != vk::Result::eSuccess &&
               acquireResult != vk::Result::eSuboptimalKHR) {
      throw std::runtime_error("failed to acquire swapchain image");
    }

    currentImageIndex = acquire.value;
    device->resetFences(inFlightFences[index]);
    auto commandBuffer = commandBuffers[index];
    vkResetCommandBuffer(commandBuffer, 0);

    return std::make_tuple(commandBuffer, currentImageIndex);
  }

  vk::Framebuffer getFramebuffer() {
    return swapchainFramebuffers[getCurrentSwapchainResourceIndex()].get();
  }

  void endRender(const std::vector<vk::CommandBuffer> &commandBuffers) {
    const auto index = getCurrentSwapchainResourceIndex();

    const auto waitDstStageMask =
        vk::PipelineStageFlagBits::eColorAttachmentOutput;
    const auto submitInfo =
        vk::SubmitInfo{}
            .setWaitSemaphores({imageAvailableSemaphores[index]})
            .setWaitDstStageMask({waitDstStageMask})
            .setCommandBuffers(commandBuffers)
            .setSignalSemaphores({renderFinishedSemaphores[index]});

    graphicsQueue.submit(submitInfo, inFlightFences[index]);

    const auto presentInfo =
        vk::PresentInfoKHR{}
            .setWaitSemaphores({renderFinishedSemaphores[index]})
            .setSwapchains({swapchain.get()})
            .setImageIndices({currentImageIndex});

    const auto presentResult = presentQueue.presentKHR(presentInfo);
    if (presentResult == vk::Result::eErrorOutOfDateKHR ||
        presentResult == vk::Result::eSuboptimalKHR || framebufferResized) {
      framebufferResized = false;
      // recreate swapchain
      throw std::runtime_error("swapchain recreation not implemented yet");
    } else if (presentResult != vk::Result::eSuccess) {
      throw std::runtime_error("failed to present swapchain image");
    }

    ++frame;
  }

  static GLFWwindow *window;
  static vk::UniqueInstance instance;
  static vk::PhysicalDevice physicalDevice;
  static vk::UniqueDevice device;

  static vk::UniqueCommandPool commandPool;

  static vk::UniqueSwapchainKHR swapchain;
  static vk::Format swapchainImageFormat;
  static vk::Extent2D swapchainExtent;

  uint32_t getCurrentSwapchainResourceIndex() {
    return frame % swapchainImageCount;
  }

  uint32_t getSwapchainImageCount() { return swapchainImageCount; }

  static vk::ImageView createImageView(vk::Image image, vk::Format format,
                                       vk::ImageAspectFlags aspectFlags,
                                       uint32_t mipLevels,
                                       const char *name = "unnamed");
  static vk::UniqueImageView
  createImageViewUnique(vk::Image image, vk::Format format,
                        vk::ImageAspectFlags aspectFlags, uint32_t mipLevels,
                        const char *name = "unnamed");

  void createFramebuffers(const vk::UniqueRenderPass &renderPass);

  static const vk::SampleCountFlagBits msaaSamples =
      vk::SampleCountFlagBits::e1;

private:
  vk::UniqueInstance
  createInstance(const char *applicationInfo, uint32_t applicationVersion,
                 const std::vector<const char *> &requiredExtensions);

  vk::UniqueDevice createDevice();
  vk::UniqueSwapchainKHR createSwapchain();
  vk::UniqueCommandPool createCommandPool();

  vk::PhysicalDevice getSuitablePhysicalDevice();
  bool isDeviceSuitable(vk::PhysicalDevice physicalDevice) const;

  UniqueSurface surface{};

  vk::Queue graphicsQueue;
  vk::Queue presentQueue;

  uint32_t graphicsQueueFamily;
  uint32_t presentQueueFamily;

  uint32_t swapchainImageCount;
  std::vector<vk::Image> swapchainImages;
  std::vector<vk::UniqueImageView> swapchainImageViews;
  std::vector<vk::UniqueFramebuffer> swapchainFramebuffers;
  uint32_t currentImageIndex = 0;
  bool framebufferResized = false;

  Image colorImage;
  vk::UniqueImageView colorImageView;
  Image depthImage;
  vk::UniqueImageView depthImageView;

  vk::Pipeline graphicsPipeline;
  vk::PipelineLayout graphicsPipelineLayout;

  uint32_t frame = 0;
  std::vector<vk::Fence> inFlightFences;
  std::vector<vk::Semaphore> imageAvailableSemaphores;
  std::vector<vk::Semaphore> renderFinishedSemaphores;
  std::vector<vk::CommandBuffer> commandBuffers;
};
} // namespace Vulking
