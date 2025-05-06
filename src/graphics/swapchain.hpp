#pragma once

#include "framebuffer.hpp"
#include "gpu.hpp"
#include <vulkan/vulkan_core.h>

class Swapchain {
public:
  GPU::SwapchainSupportDetails supportDetails;
  VkSurfaceFormatKHR format;
  VkExtent2D extent;
  VkPresentModeKHR presentMode;

  Swapchain(int width, int height);
  ~Swapchain();

  operator VkSwapchainKHR() const { return swapchain; }

private:
  VkSurfaceKHR surface;
  VkSwapchainKHR swapchain;
  std::vector<Framebuffer> framebuffers;

  VkSurfaceFormatKHR
  chooseFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);

  VkExtent2D chooseExtent(int width, int height,
                          const VkSurfaceCapabilitiesKHR &capabilities);

  VkPresentModeKHR
  choosePresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);
};
