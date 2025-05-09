#pragma once
#include "../common.hpp"
#include "Device.hpp"
#include "PhysicalDevice.hpp"
#include "Surface.hpp"
#include <optional>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace Vulking {
class SwapChain {
public:
  SwapChain(const PhysicalDevice &physicalDevice, const Device &device,
            const Surface &surface);
  void release();
  operator VkSwapchainKHR() const;

  const std::vector<VkImage> &getImages() const;
  VkFormat getFormat() const;
  VkExtent2D getExtent() const;

  void recreateSwapChain(int width, int height);

private:
  const PhysicalDevice &physicalDevice;
  const Device &device;
  const Surface &surface;

  VkSwapchainKHR swapChain{};
  std::vector<VkImage> images{};
  std::vector<VkImageView> imageViews{};
  std::vector<VkFramebuffer> framebuffers{};

  VkSwapchainKHR createSwapChain();
};
} // namespace Vulking
