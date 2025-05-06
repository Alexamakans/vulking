#include "swapchain.hpp"
#include "../common.hpp"
#include "../engine.hpp"

Swapchain::Swapchain(int width, int height)
    : surface(Engine::surface),
      supportDetails(Engine::gpu.swapchainSupportDetails),
      extent(chooseExtent(width, height, supportDetails.capabilities)),
      format(chooseFormat(supportDetails.formats)),
      presentMode(choosePresentMode(supportDetails.presentModes)) {
  const auto &caps = supportDetails.capabilities;
  uint32_t imageCount = caps.minImageCount + 1;
  if (caps.maxImageCount > 0 && imageCount > caps.maxImageCount) {
    imageCount = caps.maxImageCount;
  }

  VkSwapchainCreateInfoKHR createInfo{};
  createInfo.sType = STYPE(SWAPCHAIN_CREATE_INFO_KHR);
  createInfo.surface = surface;

  createInfo.minImageCount = imageCount;
  createInfo.imageFormat = format.format;
  createInfo.imageColorSpace = format.colorSpace;
  createInfo.imageExtent = extent;
  createInfo.imageArrayLayers = 1;
  createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  const auto &indices = Engine::gpu.queueFamilyIndices;
  uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(),
                                   indices.presentFamily.value()};

  if (indices.graphicsFamily != indices.presentFamily) {
    createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    createInfo.queueFamilyIndexCount = 2;
    createInfo.pQueueFamilyIndices = queueFamilyIndices;
  } else {
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.queueFamilyIndexCount = 0;
    createInfo.pQueueFamilyIndices = nullptr;
  }

  createInfo.preTransform =
      Engine::swapchain.supportDetails.capabilities.currentTransform;
  createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  createInfo.presentMode = Engine::swapchain.presentMode;
  createInfo.clipped = VK_TRUE;

  createInfo.oldSwapchain = VK_NULL_HANDLE;

  CHK(vkCreateSwapchainKHR(Engine::device, &createInfo, nullptr, &swapchain),
      "failed to create swap chain")

  std::vector<VkImage> images;
  vkGetSwapchainImagesKHR(Engine::device, swapchain, &imageCount, nullptr);
  images.resize(imageCount);
  vkGetSwapchainImagesKHR(Engine::device, swapchain, &imageCount,
                          images.data());

  framebuffers.resize(imageCount);
  for (size_t i = 0; i < imageCount; i++) {
  }
};

Swapchain::~Swapchain() {
  vkDestroySwapchainKHR(Engine::device, swapchain, allocator);
};

VkSurfaceFormatKHR Swapchain::chooseFormat(
    const std::vector<VkSurfaceFormatKHR> &availableFormats) {
  for (const auto &availableFormat : availableFormats) {
    if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
        availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
      return availableFormat;
    }
  }

  return availableFormats[0];
}

VkExtent2D
Swapchain::chooseExtent(int width, int height,
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

VkPresentModeKHR Swapchain::choosePresentMode(
    const std::vector<VkPresentModeKHR> &availablePresentModes) {
  for (const auto &availablePresentMode : availablePresentModes) {
    if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
      return availablePresentMode;
    }
  }

  return VK_PRESENT_MODE_FIFO_KHR;
}
