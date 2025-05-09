#include "SwapChain.hpp"
#include "Queue.hpp"
#include <ios>
#include <vulkan/vulkan_core.h>

Vulking::SwapChain::SwapChain(const PhysicalDevice &physicalDevice,
                              const Device &device, const Surface &surface)
    : physicalDevice(physicalDevice), device(device), surface(surface),
      swapChain(createSwapChain()) {}

Vulking::SwapChain::operator VkSwapchainKHR() const { return swapChain; }

const std::vector<VkImage> &Vulking::SwapChain::getImages() const {
  return images;
}

VkFormat Vulking::SwapChain::getFormat() const {
  return physicalDevice.getFormat();
}

VkExtent2D Vulking::SwapChain::getExtent() const {
  return physicalDevice.getExtent();
}

VkSwapchainKHR Vulking::SwapChain::createSwapChain() {
  // Create the swap chain
  VkSwapchainCreateInfoKHR createInfo{};
  createInfo.sType = STYPE(SWAPCHAIN_CREATE_INFO_KHR);
  createInfo.surface = surface;
  createInfo.imageFormat = getFormat();
  createInfo.imageExtent = getExtent();
  createInfo.imageArrayLayers = 1;
  createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  auto &caps = physicalDevice.getCapabilities();
  createInfo.minImageCount =
      std::min(caps.minImageCount + 1, caps.maxImageCount);

  const auto graphicsFamily =
      physicalDevice.queueFamilyIndices.graphicsFamily.value();
  const auto presentFamily =
      physicalDevice.queueFamilyIndices.presentFamily.value();
  Queue graphicsQueue(device, graphicsFamily);
  uint32_t indices[] = {graphicsFamily, presentFamily};

  if (graphicsFamily != presentFamily) {
    createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    createInfo.queueFamilyIndexCount = 2;
    createInfo.pQueueFamilyIndices = indices;
  } else {
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.queueFamilyIndexCount = 0;
    createInfo.pQueueFamilyIndices = nullptr;
  }

  createInfo.preTransform = physicalDevice.getCapabilities().currentTransform;
  createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  createInfo.presentMode = physicalDevice.getPresentMode();
  createInfo.clipped = VK_TRUE;
  createInfo.oldSwapchain = VK_NULL_HANDLE;
#define A(x) std::cout << std::format("{}: ", #x) << createInfo.x << std::endl;
  A(preTransform);
  A(presentMode);
  A(minImageCount);
  A(imageExtent.width);
  A(imageExtent.height);
  A(sType);
  A(surface);
  std::cout << "imageFormat: " << std::hex << createInfo.imageFormat
            << std::endl;
  A(imageArrayLayers);
  A(imageUsage);
#undef A

  VkSwapchainKHR swapChain{};
  CHK(vkCreateSwapchainKHR(device, &createInfo, allocator, &swapChain),
      "failed to create swap chain.");

  // Retrieve swap chain images
  uint32_t imageCount = 0;
  CHK(vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr),
      "failed to get swapchain image count");
  std::cout << "got num swap chain img: " << imageCount << std::endl;

  std::cout << images.capacity() << std::endl;
  images.resize(imageCount);
  imageViews.resize(imageCount);
  CHK(vkGetSwapchainImagesKHR(device, swapChain, &imageCount, images.data()),
      "failed to get swapchain images");

  return swapChain;
}
void Vulking::SwapChain::release() {
  for (auto framebuffer : framebuffers) {
    vkDestroyFramebuffer(device, framebuffer, allocator);
  }
  for (auto imageView : imageViews) {
    vkDestroyImageView(device, imageView, allocator);
  }
  vkDestroySwapchainKHR(device, swapChain, allocator);
}

void Vulking::SwapChain::recreateSwapChain(int width, int height) {
  CHK(vkDeviceWaitIdle(device), "failed to wait for device to be idle");
  release();
  swapChain = createSwapChain();
}
