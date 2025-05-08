#pragma once
#include "../common.hpp"
#include "Device.hpp"
#include "Image.hpp"
#include "PhysicalDevice.hpp"
#include "Surface.hpp"
#include <optional>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace Vulking {
class SwapChain {
public:
  SwapChain() = default;
  SwapChain(PhysicalDevice physicalDevice, Device device, Surface surface);
  ~SwapChain();

  operator VkSwapchainKHR() const;

  const std::vector<VkImage> &getImages() const;
  VkFormat getImageFormat() const;
  VkExtent2D getExtent() const;

private:
  PhysicalDevice physicalDevice;
  Device device;
  Surface surface;

  VkSwapchainKHR swapChain{};
  VkFormat swapChainImageFormat;
  VkExtent2D swapChainExtent;
  std::vector<VkImage> swapChainImages;

  VkSwapchainKHR createSwapChain();
};
} // namespace Vulking
