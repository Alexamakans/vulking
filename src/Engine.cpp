#include "Engine.hpp"
#include "Common.hpp"
#include "Constants.hpp"
#include "Functions.hpp"
#include "vulking/Image.hpp"

#include <cassert>
#include <iostream>
#include <set>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_handles.hpp>
#include <vulkan/vulkan_structs.hpp>

namespace Vulking {

GLFWwindow *Engine::window = nullptr;

vk::UniqueInstance Engine::instance;
vk::PhysicalDevice Engine::physicalDevice;
vk::UniqueDevice Engine::device;

vk::CommandPool Engine::commandPool;
vk::DescriptorPool Engine::descriptorPool;

vk::UniqueSwapchainKHR Engine::swapchain;
vk::Format Engine::swapchainImageFormat;
vk::Extent2D Engine::swapchainExtent;

Engine::Engine(GLFWwindow *window, const char *applicationInfo,
               uint32_t applicationVersion,
               const std::vector<const char *> &requiredExtensions) {
  Engine::window = window;
  Engine::instance =
      createInstance(applicationInfo, applicationVersion, requiredExtensions);

  VkSurfaceKHR _surface;
  VkResult result = glfwCreateWindowSurface(
      (VkInstance)getVulkanHandle(instance.get()), window, nullptr, &_surface);
  if (result != VK_SUCCESS) {
    throw std::runtime_error("failed to create window surface");
  }
  surface = ScopedSurface(instance.get(), _surface);

  Engine::physicalDevice = getSuitablePhysicalDevice();
  Engine::device = createDevice();
  DYNAMIC_DISPATCHER = vk::detail::DispatchLoaderDynamic(
      instance.get(), vkGetInstanceProcAddr, device.get(), vkGetDeviceProcAddr);

  // Move swapchain stuff to own function so we can recreate easily
  Engine::swapchain = createSwapchain();
  swapchainImages = device->getSwapchainImagesKHR(swapchain.get());

  swapchainImageViews.resize(swapchainImages.size());
  for (uint32_t i = 0; i < swapchainImages.size(); i++) {
    swapchainImageViews[i] =
        Engine::createImageViewUnique(swapchainImages[i], swapchainImageFormat,
                                      vk::ImageAspectFlagBits::eColor, 1);
  }

  Engine::colorImage =
      Image(swapchainExtent.width, swapchainExtent.height, 1, msaaSamples,
            swapchainImageFormat, vk::ImageTiling::eOptimal,
            vk::ImageUsageFlagBits::eTransientAttachment |
                vk::ImageUsageFlagBits::eColorAttachment,
            vk::MemoryPropertyFlagBits::eDeviceLocal);
  Engine::colorImageView =
      createImageViewUnique(colorImage.image.get(), swapchainImageFormat,
                            vk::ImageAspectFlagBits::eColor, 1);

  auto depthFormat = findDepthFormat();
  Engine::depthImage =
      Image(swapchainExtent.width, swapchainExtent.height, 1, msaaSamples,
            depthFormat, vk::ImageTiling::eOptimal,
            vk::ImageUsageFlagBits::eTransientAttachment |
                vk::ImageUsageFlagBits::eDepthStencilAttachment,
            vk::MemoryPropertyFlagBits::eDeviceLocal);
  Engine::depthImageView = createImageViewUnique(
      depthImage.image.get(), depthFormat, vk::ImageAspectFlagBits::eDepth, 1);
}

vk::CommandBuffer Engine::beginCommand() {
  vk::CommandBufferAllocateInfo info;
  info.setCommandPool(commandPool);
  info.setLevel(vk::CommandBufferLevel::ePrimary);
  info.setCommandBufferCount(1);
  auto commandBuffer = device->allocateCommandBuffers(info).front();

  vk::CommandBufferBeginInfo beginInfo;
  beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
  commandBuffer.begin(beginInfo);
  return commandBuffer;
}

vk::ImageView Engine::createImageView(vk::Image image, vk::Format format,
                                      vk::ImageAspectFlags aspectFlags,
                                      uint32_t mipLevels, const char *name) {
  vk::ImageViewCreateInfo info{};
  info.setImage(image)
      .setViewType(vk::ImageViewType::e2D)
      .setFormat(format)
      .setSubresourceRange(vk::ImageSubresourceRange{}
                               .setAspectMask(aspectFlags)
                               .setBaseMipLevel(0)
                               .setLevelCount(mipLevels)
                               .setBaseArrayLayer(0)
                               .setLayerCount(1));

  auto obj = device->createImageView(info);
  NAME_OBJECT(device, obj, name);
  return obj;
}

vk::UniqueImageView
Engine::createImageViewUnique(vk::Image image, vk::Format format,
                              vk::ImageAspectFlags aspectFlags,
                              uint32_t mipLevels, const char *name) {
  vk::ImageViewCreateInfo info{};
  info.setImage(image)
      .setViewType(vk::ImageViewType::e2D)
      .setFormat(format)
      .setSubresourceRange(vk::ImageSubresourceRange{}
                               .setAspectMask(aspectFlags)
                               .setBaseMipLevel(0)
                               .setLevelCount(mipLevels)
                               .setBaseArrayLayer(0)
                               .setLayerCount(1));

  auto obj = device->createImageViewUnique(info);
  NAME_OBJECT(device, obj.get(), name);
  return std::move(obj);
}

void Engine::createFramebuffers(const vk::UniqueRenderPass &renderPass) {
  // vv call this from userland
  swapchainFramebuffers.resize(swapchainImages.size());
  for (uint32_t i = 0; i < swapchainImages.size(); i++) {
    std::array<vk::ImageView, 3> attachments = {
        colorImageView.get(),
        depthImageView.get(),
        swapchainImageViews[i].get(),
    };

    auto info = vk::FramebufferCreateInfo{}
                    .setRenderPass(renderPass.get())
                    .setAttachments(attachments)
                    .setWidth(swapchainExtent.width)
                    .setHeight(swapchainExtent.height)
                    .setLayers(1);

    swapchainFramebuffers[i] = Engine::device->createFramebufferUnique(info);
  }
}

vk::UniqueInstance
Engine::createInstance(const char *applicationInfo, uint32_t applicationVersion,
                       const std::vector<const char *> &requiredExtensions) {
  vk::ApplicationInfo appInfo{};
  appInfo.setPApplicationName(applicationInfo);
  appInfo.setApplicationVersion(applicationVersion);
  appInfo.setPEngineName("Vulking");
  appInfo.setEngineVersion(vk::makeApiVersion(0, 0, 0, 1));
  appInfo.setApiVersion(vk::ApiVersion14);
  vk::InstanceCreateInfo info{};
  info.setPApplicationInfo(&appInfo);
  info.setPApplicationInfo(&appInfo);
  auto extensions = requiredExtensions;
  if (enableValidationLayers) {
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  }
  info.setPEnabledExtensionNames(extensions);
  auto _instance = vk::createInstanceUnique(info, ALLOCATOR);
  return _instance;
}

vk::PhysicalDevice Engine::getSuitablePhysicalDevice() {
  auto physicalDevices = instance->enumeratePhysicalDevices();
  for (const auto &physicalDevice : physicalDevices) {
    if (isDeviceSuitable(physicalDevice)) {
      return physicalDevice;
    }
  }

  throw std::runtime_error("failed to find a suitable GPU.");
}

bool Engine::isDeviceSuitable(vk::PhysicalDevice physicalDevice) const {
  assert(physicalDevice != VK_NULL_HANDLE);
  // This should be extended based on required features, extensions, and queue
  // families.
  auto props = physicalDevice.getProperties();

  std::cout << "GPU Device properties:\n";
  std::cout << "\t vendorID = " << props.vendorID << "\n";
  std::cout << "\t deviceID = " << props.deviceID << "\n";
  std::cout << "\t apiVersion = " << props.apiVersion << "\n";
  std::cout << "\t driverVersion = " << props.driverVersion << "\n";
  std::cout << "\t deviceName = " << props.deviceName << std::endl;

  return props.deviceType == vk::PhysicalDeviceType::eDiscreteGpu ||
         props.deviceType == vk::PhysicalDeviceType::eIntegratedGpu;
}

vk::UniqueDevice Engine::createDevice() {
  vk::ArrayProxy<float> queuePriorities = 1.0f;

  std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
  std::set<uint32_t> uniqueFamilies = {graphicsQueueFamily, presentQueueFamily};

  for (uint32_t queueFamily : uniqueFamilies) {
    vk::DeviceQueueCreateInfo queueCreateInfo{};
    queueCreateInfo.setQueueFamilyIndex(queueFamily);
    queueCreateInfo.setQueuePriorities(queuePriorities);
    queueCreateInfos.push_back(queueCreateInfo);
  }

  vk::PhysicalDeviceFeatures deviceFeatures{};
  deviceFeatures.samplerAnisotropy = VK_TRUE;
  if (ENABLE_SAMPLE_SHADING) {
    deviceFeatures.sampleRateShading = VK_TRUE;
  }

  vk::DeviceCreateInfo createInfo{};
  createInfo.setQueueCreateInfos(queueCreateInfos);
  createInfo.setPEnabledFeatures(&deviceFeatures);
  createInfo.setPEnabledExtensionNames(DEVICE_EXTENSIONS);

  return physicalDevice.createDeviceUnique(createInfo, ALLOCATOR);
}

vk::SurfaceFormatKHR chooseSwapSurfaceFormat(
    const std::vector<vk::SurfaceFormatKHR> &availableFormats);

vk::PresentModeKHR chooseSwapPresentMode(
    const std::vector<vk::PresentModeKHR> &availablePresentModes);

vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR &capabilities);

vk::UniqueSwapchainKHR Engine::createSwapchain() {
  auto caps = physicalDevice.getSurfaceCapabilitiesKHR(surface);
  auto formats = physicalDevice.getSurfaceFormatsKHR(surface);
  auto presentModes = physicalDevice.getSurfacePresentModesKHR(surface);

  auto extent = chooseSwapExtent(caps);
  auto format = chooseSwapSurfaceFormat(formats);
  auto imageCount = caps.minImageCount + 1;
  if (caps.maxImageCount > 0 && imageCount > caps.maxImageCount) {
    imageCount = caps.maxImageCount;
  }
  Engine::swapchainImageCount = imageCount;

  vk::SwapchainCreateInfoKHR info{};
  info.setImageFormat(format.format)
      .setImageColorSpace(format.colorSpace)
      .setSurface(surface)
      .setPresentMode(chooseSwapPresentMode(presentModes))
      .setMinImageCount(imageCount)
      .setImageExtent(extent)
      .setImageArrayLayers(1)
      .setImageUsage(vk::ImageUsageFlagBits::eColorAttachment);

  if (graphicsQueueFamily != presentQueueFamily) {
    info.setImageSharingMode(vk::SharingMode::eConcurrent);
    std::array<uint32_t, 2> indices{graphicsQueueFamily, presentQueueFamily};
    info.setQueueFamilyIndices(indices);
  } else {
    info.setImageSharingMode(vk::SharingMode::eExclusive);
  }

  info.setPreTransform(caps.currentTransform);
  info.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque);
  info.setClipped(vk::True);
  info.setOldSwapchain(VK_NULL_HANDLE);

  Engine::swapchainImageFormat = format.format;
  Engine::swapchainExtent = extent;

  return device->createSwapchainKHRUnique(info);
}

vk::SurfaceFormatKHR chooseSwapSurfaceFormat(
    const std::vector<vk::SurfaceFormatKHR> &availableFormats) {
  for (const auto &availableFormat : availableFormats) {
    if (availableFormat.format == vk::Format::eB8G8R8A8Srgb &&
        availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
      return availableFormat;
    }
  }

  return availableFormats[0];
}

vk::PresentModeKHR chooseSwapPresentMode(
    const std::vector<vk::PresentModeKHR> &availablePresentModes) {
  for (const auto &availablePresentMode : availablePresentModes) {
    if (availablePresentMode == vk::PresentModeKHR::eMailbox) {
      return availablePresentMode;
    }
  }

  return vk::PresentModeKHR::eFifo;
}

vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR &capabilities) {
  if (capabilities.currentExtent.width !=
      std::numeric_limits<uint32_t>::max()) {
    return capabilities.currentExtent;
  } else {
    int width, height;
    glfwGetFramebufferSize(Engine::window, &width, &height);

    vk::Extent2D actualExtent{static_cast<uint32_t>(width),
                              static_cast<uint32_t>(height)};

    actualExtent.width =
        std::clamp(actualExtent.width, capabilities.minImageExtent.width,
                   capabilities.maxImageExtent.width);
    actualExtent.height =
        std::clamp(actualExtent.height, capabilities.minImageExtent.height,
                   capabilities.maxImageExtent.height);

    return actualExtent;
  }
}
} // namespace Vulking
