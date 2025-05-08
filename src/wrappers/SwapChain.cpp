#include "SwapChain.hpp"

Vulking::SwapChain::SwapChain(Device device, Surface surface)
    : device(device), surface(surface) {
  createSwapChain(device, surface);
}

Vulking::SwapChain::~SwapChain() {
  if (swapChain) {
    vkDestroySwapchainKHR(device, swapChain, allocator);
  }
}

Vulking::SwapChain::operator VkSwapchainKHR() const { return swapChain; }

const std::vector<VkImage> &Vulking::SwapChain::getImages() const {
  return swapChainImages;
}

VkFormat Vulking::SwapChain::getImageFormat() const {
  return swapChainImageFormat;
}

VkExtent2D Vulking::SwapChain::getExtent() const { return swapChainExtent; }

void Vulking::SwapChain::createSwapChain(GPU gpu, Surface surface) {
  // Get surface capabilities
  VkSurfaceCapabilitiesKHR capabilities;
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu, surface, &capabilities);

  // Get surface formats
  uint32_t formatCount = 0;
  vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &formatCount, nullptr);
  std::vector<VkSurfaceFormatKHR> surfaceFormats(formatCount);
  vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &formatCount,
                                       surfaceFormats.data());

  VkSurfaceFormatKHR surfaceFormat = surfaceFormats[0];
  swapChainImageFormat = surfaceFormat.format;

  // Get surface present modes
  uint32_t presentModeCount = 0;
  vkGetPhysicalDeviceSurfacePresentModesKHR(gpu, surface, &presentModeCount,
                                            nullptr);
  std::vector<VkPresentModeKHR> presentModes(presentModeCount);
  vkGetPhysicalDeviceSurfacePresentModesKHR(gpu, surface, &presentModeCount,
                                            presentModes.data());

  // Choose present mode (FIFO is guaranteed to be supported)
  VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;

  // Choose swap chain extent (size of the swap chain)
  swapChainExtent = capabilities.currentExtent;

  // Create the swap chain
  VkSwapchainCreateInfoKHR createInfo{};
  createInfo.sType = STYPE(SWAPCHAIN_CREATE_INFO_KHR);
  createInfo.surface = surface;
  createInfo.minImageCount = capabilities.minImageCount + 1;
  createInfo.imageFormat = swapChainImageFormat;
  createInfo.imageExtent = swapChainExtent;
  createInfo.imageArrayLayers = 1;
  createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  Queue graphicsQueue(device, device.getGraphicsQueueFamily());
  uint32_t indices[] = {device.getGraphicsQueueFamily(),
                        device.getPresentQueueFamily()};

  if (device.getGraphicsQueueFamily() != device.getPresentQueueFamily()) {
    createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    createInfo.queueFamilyIndexCount = 2;
    createInfo.pQueueFamilyIndices = indices;
  } else {
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.queueFamilyIndexCount = 0;
    createInfo.pQueueFamilyIndices = nullptr;
  }

  createInfo.preTransform = capabilities.currentTransform;
  createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  createInfo.presentMode = presentMode;
  createInfo.clipped = VK_TRUE;
  createInfo.oldSwapchain = VK_NULL_HANDLE;

  CHK(vkCreateSwapchainKHR(device, &createInfo, allocator, &swapChain),
      "Failed to create swap chain.");

  // Retrieve swap chain images
  uint32_t imageCount = 0;
  vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
  swapChainImages.resize(imageCount);
  vkGetSwapchainImagesKHR(device, swapChain, &imageCount,
                          swapChainImages.data());
}
