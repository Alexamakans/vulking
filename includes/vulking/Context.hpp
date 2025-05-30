#pragma once

#include "Common.hpp"
#include "Image.hpp"
#include "UniqueSurface.hpp"

namespace Vulking {
struct Swapchain {
  Swapchain() {}
  Swapchain(const Swapchain &) = delete;
  Swapchain &operator=(const Swapchain &) = delete;
  Swapchain(Swapchain &&) = delete;
  Swapchain &operator=(Swapchain &&) = delete;

  vk::UniqueSwapchainKHR handle;
  uint32_t imageCount;
  vk::Format imageFormat;
  vk::Extent2D extent;
  Image color;
  vk::UniqueImageView colorView;
  Image depth;
  vk::UniqueImageView depthView;

  std::vector<vk::Image> images;
  std::vector<vk::UniqueImageView> views;
  std::vector<vk::UniqueFramebuffer> framebuffers;

  uint32_t currentImageIndex;
  bool framebufferResized;

  void createFramebuffers(const vk::RenderPass &renderPass);
  vk::Framebuffer getFramebuffer();
  uint32_t getCurrentResourceIndex();
};

struct Context {
  Context()
      : window(nullptr), msaaSamples(vk::SampleCountFlagBits::e1), frame(0) {}
  Context(const Context &) = delete;
  Context &operator=(const Context &) = delete;
  Context(Context &&) = delete;
  Context &operator=(Context &&other) = delete;

  GLFWwindow *window;
  vk::UniqueInstance instance;
  UniqueSurface surface;
  vk::PhysicalDevice physicalDevice;
  vk::UniqueDevice device;

  Swapchain swapchain;

  vk::UniqueCommandPool commandPool;

  std::vector<vk::UniqueCommandBuffer> commandBuffers;
  std::vector<vk::UniqueFence> inFlightFences;
  std::vector<vk::UniqueSemaphore> imageAvailableSemaphores;
  std::vector<vk::UniqueSemaphore> renderFinishedSemaphores;

  vk::Queue graphicsQueue;
  uint32_t graphicsQueueFamily;
  vk::Queue presentQueue;
  uint32_t presentQueueFamily;

  vk::SampleCountFlagBits msaaSamples = vk::SampleCountFlagBits::e1;

  uint32_t frame;

  vk::CommandBuffer beginCommand(const char *name = "unnamed");
  void endAndSubmitGraphicsCommand(vk::CommandBuffer &&cmd);

  /* tuple<command buffer to populate, current swapchain resourceIndex> */
  std::optional<std::tuple<vk::CommandBuffer, uint32_t>> beginRender();

  void endRender(const std::vector<vk::CommandBuffer> &commandBuffers);

  vk::ImageView createImageView(vk::Image image, vk::Format format,
                                vk::ImageAspectFlags aspectFlags,
                                uint32_t mipLevels,
                                const char *name = "unnamed");
  vk::UniqueImageView createImageViewUnique(vk::Image image, vk::Format format,
                                            vk::ImageAspectFlags aspectFlags,
                                            uint32_t mipLevels,
                                            const char *name = "unnamed");
};
} // namespace Vulking
