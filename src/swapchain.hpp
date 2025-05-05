#pragma once

#include "common.hpp"
#include "gpu.hpp"
#include <vulkan/vulkan_core.h>

class Swapchain {
public:
  Swapchain(VkDevice device, int width, int height,
            GPU::SwapchainSupportDetails supportDetails, VkSurfaceKHR surface)
      : surface(surface), supportDetails(supportDetails),
        extent(chooseExtent(width, height, supportDetails.capabilities)),
        format(chooseFormat(supportDetails.formats)),
        presentMode(choosePresentMode(supportDetails.presentModes)) {};
  ~Swapchain() { vkDestroySwapchainKHR(device, swapchain, allocator); };

private:
  VkDevice device;
  VkSurfaceKHR surface;
  VkSwapchainKHR swapchain;

  GPU::SwapchainSupportDetails supportDetails;
  VkSurfaceFormatKHR format;
  VkExtent2D extent;
  VkPresentModeKHR presentMode;

  VkSurfaceFormatKHR
  chooseFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats) {
    for (const auto &availableFormat : availableFormats) {
      if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
          availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
        return availableFormat;
      }
    }

    return availableFormats[0];
  }

  VkExtent2D chooseExtent(int width, int height,
                          const VkSurfaceCapabilitiesKHR &capabilities) {
    if (capabilities.currentExtent.width !=
        std::numeric_limits<uint32_t>::max()) {
      return capabilities.currentExtent;
    } else {
      VkExtent2D actualExtent = {
          static_cast<uint32_t>(width),
          static_cast<uint32_t>(height),
      };

      actualExtent.width =
          std::clamp(actualExtent.width, capabilities.minImageExtent.width,
                     capabilities.maxImageExtent.width);
      actualExtent.height =
          std::clamp(actualExtent.height, capabilities.minImageExtent.height,
                     capabilities.maxImageExtent.height);
      return actualExtent;
    }
  }

  VkPresentModeKHR choosePresentMode(
      const std::vector<VkPresentModeKHR> &availablePresentModes) {
    for (const auto &availablePresentMode : availablePresentModes) {
      if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
        return availablePresentMode;
      }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
  }
};
