#include "SwapChain.hpp"

SwapChain::SwapChain(VkDevice dev, VkSurfaceKHR surf, const Device &device)
    : device(dev), surface(surf) {
  createSwapChain(device, surface);
}

SwapChain::~SwapChain() {
  if (swapChain) {
    vkDestroySwapchainKHR(device, swapChain, allocator);
  }
}

SwapChain::operator VkSwapchainKHR() const { return swapChain; }

const std::vector<VkImage> &SwapChain::getImages() const {
  return swapChainImages;
}

VkFormat SwapChain::getImageFormat() const { return swapChainImageFormat; }

VkExtent2D SwapChain::getExtent() const { return swapChainExtent; }

void SwapChain::createSwapChain(const Device &device, VkSurfaceKHR surface) {
  // Get surface capabilities
  VkSurfaceCapabilitiesKHR capabilities;
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface,
                                            &capabilities);

  // Get surface formats
  uint32_t formatCount = 0;
  vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface,
                                       &formatCount, nullptr);
  std::vector<VkSurfaceFormatKHR> surfaceFormats(formatCount);
  vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface,
                                       &formatCount, surfaceFormats.data());

  VkSurfaceFormatKHR surfaceFormat = surfaceFormats[0];
  swapChainImageFormat = surfaceFormat.format;

  // Get surface present modes
  uint32_t presentModeCount = 0;
  vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface,
                                            &presentModeCount, nullptr);
  std::vector<VkPresentModeKHR> presentModes(presentModeCount);
  vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount,
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
