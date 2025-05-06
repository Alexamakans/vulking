#pragma once

#include "swapchain.hpp"
#include <vulkan/vulkan_core.h>

class SwapchainImage {
public:
  SwapchainImage(Swapchain swapchain, VkImage image);
  ~SwapchainImage();

private:
  VkImage image;
  VkImageView view;
};
