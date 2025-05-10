#pragma once
#include "../common.hpp"
#include "Device.hpp"
#include "PhysicalDevice.hpp"
#include "RenderPass.hpp"
#include "Surface.hpp"
#include <optional>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace Vulking {
class SwapChain {
public:
  SwapChain(const PhysicalDevice &physicalDevice, const Device &device,
            const Surface &surface, const RenderPass &renderPass);
  void release();
  operator VkSwapchainKHR() const;

  const std::vector<VkImage> &getImages() const;
  VkFormat getFormat() const;
  VkExtent2D getExtent() const;

  void recreateSwapChain();

private:
  const PhysicalDevice &physicalDevice;
  const Device &device;
  const Surface &surface;
  const RenderPass &renderPass;

  VkSwapchainKHR swapChain{};
  // These are images retrieved from the swapchain, do not require deallocation.
  // They correspond to the surface image that we eventually present to the
  // screen. I.e. it's the output image.
  std::vector<VkImage> images{};
  std::vector<VkImageView> imageViews{};

  VkImage colorImage;
  VkDeviceMemory colorImageMemory;
  VkImageView colorImageView;

  VkImage depthImage;
  VkDeviceMemory depthImageMemory;
  VkImageView depthImageView;

  // Binds together color, depth and swapchain image for the render pass to
  // render into.
  std::vector<VkFramebuffer> framebuffers{};

  VkSwapchainKHR createSwapChain();
  void fetchSwapChainImages();
  void createSwapchainImageViews();
  void createColorResources();
  void createDepthResources();
  void createFramebuffers();
};
} // namespace Vulking
