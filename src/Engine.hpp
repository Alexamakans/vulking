#pragma once

#include "Common.hpp"
#include "Image.hpp"
#include "ScopedSurface.hpp"

#define VULKING_MAKE_VERSION(major, minor, patch)                              \
  VK_MAKE_VERSION(major, minor, patch)

namespace Vulking {
class Engine {
public:
  Engine(GLFWwindow *window, const char *applicationInfo,
         uint32_t applicationVersion,
         const std::vector<const char *> &requiredExtensions);

  static vk::CommandBuffer beginCommand();

  static GLFWwindow *window;
  static vk::UniqueInstance instance;
  static vk::PhysicalDevice physicalDevice;
  static vk::UniqueDevice device;

  static vk::CommandPool commandPool;
  static vk::DescriptorPool descriptorPool;

  static vk::UniqueSwapchainKHR swapchain;
  static vk::Format swapchainImageFormat;
  static vk::Extent2D swapchainExtent;

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

  vk::PhysicalDevice getSuitablePhysicalDevice();
  bool isDeviceSuitable(vk::PhysicalDevice physicalDevice) const;

  ScopedSurface surface;

  uint32_t graphicsQueueFamily;
  uint32_t presentQueueFamily;

  uint32_t swapchainImageCount;
  std::vector<vk::Image> swapchainImages;
  std::vector<vk::UniqueImageView> swapchainImageViews;
  std::vector<vk::UniqueFramebuffer> swapchainFramebuffers;

  Image colorImage;
  vk::UniqueImageView colorImageView;
  Image depthImage;
  vk::UniqueImageView depthImageView;

  vk::Pipeline graphicsPipeline;
  vk::PipelineLayout graphicsPipelineLayout;
};
} // namespace Vulking
