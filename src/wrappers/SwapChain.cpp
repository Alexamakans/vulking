#include "SwapChain.hpp"
#include "../helpers/VulkingUtil.hpp"
#include "Queue.hpp"
#include <ios>
#include <vulkan/vulkan_core.h>

Vulking::SwapChain::SwapChain(const PhysicalDevice &physicalDevice,
                              const Device &device, const Surface &surface,
                              const RenderPass &renderPass)
    : physicalDevice(physicalDevice), device(device), surface(surface),
      swapChain(createSwapChain()), renderPass(renderPass) {
  fetchSwapChainImages();
  createSwapchainImageViews();
  createColorResources();
  createDepthResources();
  createFramebuffers();
}

void Vulking::SwapChain::release() {
  vkDestroyImageView(device, depthImageView, allocator);
  vkDestroyImage(device, depthImage, allocator);
  vkFreeMemory(device, depthImageMemory, allocator);

  vkDestroyImageView(device, colorImageView, allocator);
  vkDestroyImage(device, colorImage, allocator);
  vkFreeMemory(device, colorImageMemory, allocator);

  for (auto framebuffer : framebuffers) {
    vkDestroyFramebuffer(device, framebuffer, allocator);
  }

  for (auto imageView : imageViews) {
    vkDestroyImageView(device, imageView, allocator);
  }

  vkDestroySwapchainKHR(device, swapChain, allocator);
}

Vulking::SwapChain::operator VkSwapchainKHR() const { return swapChain; }

const std::vector<VkImage> &Vulking::SwapChain::getImages() const {
  return images;
}

VkFormat Vulking::SwapChain::getFormat() const {
  return physicalDevice.getFormat();
}

VkFormat Vulking::SwapChain::getDepthFormat() const {
  return physicalDevice.getDepthFormat();
}

VkExtent2D Vulking::SwapChain::getExtent() const {
  return physicalDevice.getExtent();
}

VkFramebuffer Vulking::SwapChain::getFramebuffer(uint32_t index) const {
  return framebuffers.at(index);
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

  VkSwapchainKHR swapChain{};
  CHK(vkCreateSwapchainKHR(device, &createInfo, allocator, &swapChain),
      "failed to create swap chain.");

  return swapChain;
}

void Vulking::SwapChain::fetchSwapChainImages() {
  // Retrieve swap chain images
  uint32_t imageCount = 0;
  CHK(vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr),
      "failed to get swapchain image count");
  std::cout << "got num swap chain img: " << imageCount << std::endl;

  images.resize(imageCount);
  CHK(vkGetSwapchainImagesKHR(device, swapChain, &imageCount, images.data()),
      "failed to get swapchain images");
}

void Vulking::SwapChain::createSwapchainImageViews() {
  imageViews.resize(images.size());

  for (uint32_t i = 0; i < images.size(); i++) {
    imageViews[i] = VulkingUtil::createImageView(device, images[i], getFormat(),
                                                 VK_IMAGE_ASPECT_COLOR_BIT, 1);
  }
}

void Vulking::SwapChain::createColorResources() {
  const auto extent = getExtent();
  const auto msaaSamples = physicalDevice.getMsaaSamples();
  VulkingUtil::createImage(physicalDevice, device, extent.width, extent.height,
                           1, msaaSamples, getFormat(), VK_IMAGE_TILING_OPTIMAL,
                           VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT |
                               VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                           VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, colorImage,
                           colorImageMemory);
  colorImageView = VulkingUtil::createImageView(device, colorImage, getFormat(),
                                                VK_IMAGE_ASPECT_COLOR_BIT, 1);
}

void Vulking::SwapChain::createDepthResources() {
  const auto extent = getExtent();
  const auto msaaSamples = physicalDevice.getMsaaSamples();
  VulkingUtil::createImage(
      physicalDevice, device, extent.width, extent.height, 1, msaaSamples,
      getDepthFormat(), VK_IMAGE_TILING_OPTIMAL,
      VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage, depthImageMemory);
  depthImageView = VulkingUtil::createImageView(
      device, depthImage, getDepthFormat(), VK_IMAGE_ASPECT_DEPTH_BIT, 1);
}

void Vulking::SwapChain::createFramebuffers() {
  framebuffers.resize(imageViews.size());

  for (size_t i = 0; i < imageViews.size(); i++) {
    std::array<VkImageView, 3> attachments = {
        colorImageView,
        depthImageView,
        imageViews[i],
    };

    VkFramebufferCreateInfo framebufferInfo{};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = renderPass;
    framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    framebufferInfo.pAttachments = attachments.data();
    framebufferInfo.width = getExtent().width;
    framebufferInfo.height = getExtent().height;
    framebufferInfo.layers = 1;

    if (vkCreateFramebuffer(device, &framebufferInfo, nullptr,
                            &framebuffers[i]) != VK_SUCCESS) {
      throw std::runtime_error("failed to create framebuffer");
    }
  }
}

void Vulking::SwapChain::recreate() {
  CHK(vkDeviceWaitIdle(device), "failed to wait for device to be idle");
  release();
  swapChain = createSwapChain();
  fetchSwapChainImages();
  createSwapchainImageViews();
  createColorResources();
  createDepthResources();
  createFramebuffers();
}
