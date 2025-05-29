#pragma once

#include "Common.hpp"
#include "Image.hpp"
#include "UniqueSurface.hpp"

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
  std::optional<std::tuple<vk::CommandBuffer, uint32_t>> beginRender();

  vk::Framebuffer getFramebuffer();

  void endRender(const std::vector<vk::CommandBuffer> &commandBuffers);

  static GLFWwindow *window;
  // static + unique is not great, move all static stuff to instance properties
  // and expose some other way
  static vk::UniqueInstance instance;
  static vk::PhysicalDevice physicalDevice;
  static vk::UniqueDevice device;

  static vk::UniqueCommandPool commandPool;

  static vk::UniqueSwapchainKHR swapchain;
  static vk::Format swapchainImageFormat;
  static vk::Extent2D swapchainExtent;

  uint32_t getCurrentSwapchainResourceIndex();

  uint32_t getSwapchainImageCount();

  static vk::ImageView createImageView(vk::Image image, vk::Format format,
                                       vk::ImageAspectFlags aspectFlags,
                                       uint32_t mipLevels,
                                       const char *name = "unnamed");
  static vk::UniqueImageView
  createImageViewUnique(vk::Image image, vk::Format format,
                        vk::ImageAspectFlags aspectFlags, uint32_t mipLevels,
                        const char *name = "unnamed");

  void createFramebuffers(const vk::UniqueRenderPass &renderPass);

  static vk::SampleCountFlagBits msaaSamples;

  static void endAndSubmitGraphicsCommand(vk::CommandBuffer &&cmd);

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

  static vk::Queue graphicsQueue;
  static vk::Queue presentQueue;

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

  uint32_t frame = 0;

  // TODO: Create these resources
  std::vector<vk::UniqueCommandBuffer> commandBuffers;
  std::vector<vk::UniqueFence> inFlightFences;
  std::vector<vk::UniqueSemaphore> imageAvailableSemaphores;
  std::vector<vk::UniqueSemaphore> renderFinishedSemaphores;
};
} // namespace Vulking
