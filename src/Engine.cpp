#include "Engine.hpp"
#include "Common.hpp"
#include "Constants.hpp"
#include "Functions.hpp"
#include "UniqueSurface.hpp"
#include "vulking/Image.hpp"

#include <cassert>
#include <iostream>
#include <optional>
#include <ranges>
#include <set>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_handles.hpp>

struct QueueFamilyIndices {
  std::optional<uint32_t> graphicsFamily;
  std::optional<uint32_t> presentFamily;

  bool isComplete() {
    return graphicsFamily.has_value() && presentFamily.has_value();
  }
};

QueueFamilyIndices findQueueFamilies(vk::PhysicalDevice physicalDevice,
                                     vk::SurfaceKHR surface);

namespace Vulking {

GLFWwindow *Engine::window = nullptr;

vk::UniqueInstance Engine::instance;
vk::PhysicalDevice Engine::physicalDevice;
vk::UniqueDevice Engine::device;

vk::UniqueCommandPool Engine::commandPool;

vk::UniqueSwapchainKHR Engine::swapchain;
vk::Format Engine::swapchainImageFormat;
vk::Extent2D Engine::swapchainExtent;

vk::SampleCountFlagBits Engine::msaaSamples = vk::SampleCountFlagBits::e1;

vk::Queue Engine::graphicsQueue;
vk::Queue Engine::presentQueue;

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
  surface = std::move(UniqueSurface(instance.get(), vk::SurfaceKHR(_surface)));

  Engine::physicalDevice = getSuitablePhysicalDevice();
  Engine::device = createDevice();

  Engine::commandPool = createCommandPool();

  // Move swapchain stuff to own function so we can recreate easily
  Engine::swapchain = createSwapchain();
  swapchainImages = device->getSwapchainImagesKHR(swapchain.get());

  swapchainImageViews.resize(swapchainImages.size());
  for (uint32_t i = 0; i < swapchainImages.size(); i++) {
    swapchainImageViews[i] =
        Engine::createImageViewUnique(swapchainImages[i], swapchainImageFormat,
                                      vk::ImageAspectFlagBits::eColor, 1);
  }

  // color image (extract later)
  {
    colorImage =
        Image(swapchainExtent.width, swapchainExtent.height, 1, msaaSamples,
              swapchainImageFormat, vk::ImageTiling::eOptimal,
              vk::ImageUsageFlagBits::eTransientAttachment |
                  vk::ImageUsageFlagBits::eColorAttachment,
              vk::MemoryPropertyFlagBits::eDeviceLocal);
    colorImageView =
        createImageViewUnique(colorImage.image.get(), swapchainImageFormat,
                              vk::ImageAspectFlagBits::eColor, 1);
  }

  // depth image (extract later)
  {
    const auto depthFormat = findDepthFormat();
    depthImage = Image(swapchainExtent.width, swapchainExtent.height, 1,
                       msaaSamples, depthFormat, vk::ImageTiling::eOptimal,
                       vk::ImageUsageFlagBits::eTransientAttachment |
                           vk::ImageUsageFlagBits::eDepthStencilAttachment,
                       vk::MemoryPropertyFlagBits::eDeviceLocal);
    depthImageView = createImageViewUnique(depthImage.image.get(), depthFormat,
                                           vk::ImageAspectFlagBits::eDepth, 1);
  }

  // command buffers (extract later)
  {
    commandBuffers = device->allocateCommandBuffersUnique(
        vk::CommandBufferAllocateInfo()
            .setCommandPool(commandPool.get())
            .setLevel(vk::CommandBufferLevel::ePrimary)
            .setCommandBufferCount(swapchainImageCount));
    for (const auto &[i, cmd] : std::ranges::views::enumerate(commandBuffers)) {
      NAME_OBJECT(device, cmd.get(),
                  std::format("engine_command_buffer_{}", i));
    }
  }

  // sync objects (extract later)
  {
    // Create the fence signaled to simplify our sync loop in begin/end render.
    const auto fenceInfo =
        vk::FenceCreateInfo().setFlags(vk::FenceCreateFlagBits::eSignaled);
    inFlightFences.resize(swapchainImageCount);

    const auto semaphoreInfo = vk::SemaphoreCreateInfo();
    imageAvailableSemaphores.resize(swapchainImageCount);
    renderFinishedSemaphores.resize(swapchainImageCount);

    for (uint32_t i = 0; i < swapchainImageCount; i++) {
      inFlightFences[i] = device->createFenceUnique(fenceInfo);
      imageAvailableSemaphores[i] =
          device->createSemaphoreUnique(semaphoreInfo);
      renderFinishedSemaphores[i] =
          device->createSemaphoreUnique(semaphoreInfo);
    }
  }
}

vk::CommandBuffer Engine::beginCommand() {
  vk::CommandBufferAllocateInfo info;
  info.setCommandPool(commandPool.get());
  info.setLevel(vk::CommandBufferLevel::ePrimary);
  info.setCommandBufferCount(1);
  auto commandBuffer = device->allocateCommandBuffers(info).front();

  vk::CommandBufferBeginInfo beginInfo;
  beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
  commandBuffer.begin(beginInfo);
  return commandBuffer;
}

std::optional<std::tuple<vk::CommandBuffer, uint32_t>> Engine::beginRender() {
  const auto index = getCurrentSwapchainResourceIndex();
  const auto waitFenceResult =
      device->waitForFences(inFlightFences[index].get(), vk::True, UINT64_MAX);
  if (waitFenceResult == vk::Result::eErrorDeviceLost) {
    throw std::runtime_error("device lost");
  }

  const auto acquire =
      device->acquireNextImageKHR(Engine::swapchain.get(), UINT64_MAX,
                                  imageAvailableSemaphores[index].get());
  const auto acquireResult = acquire.result;

  if (acquireResult == vk::Result::eErrorOutOfDateKHR) {
    // recreate swapchain
    throw std::runtime_error("swapchain recreation not implemented yet");
    return std::nullopt;
  } else if (acquireResult != vk::Result::eSuccess &&
             acquireResult != vk::Result::eSuboptimalKHR) {
    throw std::runtime_error("failed to acquire swapchain image");
  }

  currentImageIndex = acquire.value;
  device->resetFences(inFlightFences[index].get());
  commandBuffers[index]->reset();

  return std::make_tuple(commandBuffers[index].get(), currentImageIndex);
}

vk::Framebuffer Engine::getFramebuffer() {
  return swapchainFramebuffers[getCurrentSwapchainResourceIndex()].get();
}

void Engine::endRender(const std::vector<vk::CommandBuffer> &commandBuffers) {
  const auto index = getCurrentSwapchainResourceIndex();

  std::vector<vk::PipelineStageFlags> waitDstStageMask{
      vk::PipelineStageFlagBits::eColorAttachmentOutput};
  const auto submitInfo =
      vk::SubmitInfo()
          .setWaitSemaphores({imageAvailableSemaphores[index].get()})
          .setWaitDstStageMask(waitDstStageMask)
          .setCommandBuffers(commandBuffers)
          .setSignalSemaphores({renderFinishedSemaphores[index].get()});

  graphicsQueue.submit(submitInfo, inFlightFences[index].get());

  const auto presentInfo =
      vk::PresentInfoKHR{}
          .setWaitSemaphores({renderFinishedSemaphores[index].get()})
          .setSwapchains({swapchain.get()})
          .setImageIndices({currentImageIndex});

  const auto presentResult = presentQueue.presentKHR(presentInfo);
  if (presentResult == vk::Result::eErrorOutOfDateKHR ||
      presentResult == vk::Result::eSuboptimalKHR || framebufferResized) {
    framebufferResized = false;
    // recreate swapchain
    throw std::runtime_error("swapchain recreation not implemented yet");
  } else if (presentResult != vk::Result::eSuccess) {
    throw std::runtime_error("failed to present swapchain image");
  }

  ++frame;
}

uint32_t Engine::getCurrentSwapchainResourceIndex() {
  return frame % swapchainImageCount;
}

uint32_t Engine::getSwapchainImageCount() { return swapchainImageCount; }

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

void Engine::endAndSubmitGraphicsCommand(vk::CommandBuffer&& cmd) {
  cmd.end();
  graphicsQueue.submit(vk::SubmitInfo().setCommandBuffers(cmd));
  device->waitIdle();
  device->freeCommandBuffers(commandPool.get(), cmd);
}

vk::UniqueInstance
Engine::createInstance(const char *applicationInfo, uint32_t applicationVersion,
                       const std::vector<const char *> &requiredExtensions) {
  vk::ApplicationInfo appInfo{};
  appInfo.setPApplicationName(applicationInfo);
  appInfo.setApplicationVersion(applicationVersion);
  appInfo.setPEngineName("Vulking");
  appInfo.setEngineVersion(vk::makeApiVersion(0, 0, 0, 1));
  appInfo.setApiVersion(vk::ApiVersion13);
  vk::InstanceCreateInfo info{};
  info.setPApplicationInfo(&appInfo);
  info.setPApplicationInfo(&appInfo);
  auto extensions = requiredExtensions;
  if (enableValidationLayers) {
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  }
  info.setPEnabledExtensionNames(extensions);
  auto _instance = vk::createInstanceUnique(info);
  return _instance;
}

vk::PhysicalDevice Engine::getSuitablePhysicalDevice() {
  auto physicalDevices = instance->enumeratePhysicalDevices();
  for (const auto &physicalDevice : physicalDevices) {
    if (isDeviceSuitable(physicalDevice)) {
      const auto props = physicalDevice.getProperties();
      const auto counts = props.limits.framebufferColorSampleCounts &
                          props.limits.framebufferDepthSampleCounts;
      const std::array<vk::SampleCountFlagBits, 6> sampleCounts = {
          vk::SampleCountFlagBits::e64, vk::SampleCountFlagBits::e32,
          vk::SampleCountFlagBits::e16, vk::SampleCountFlagBits::e8,
          vk::SampleCountFlagBits::e4,  vk::SampleCountFlagBits::e2,
      };

      for (const auto sampleCount : sampleCounts) {
        if (counts & sampleCount) {
          msaaSamples = sampleCount;
          break;
        }
      }

      if (msaaSamples == vk::SampleCountFlagBits{}) {
        // validation gets angry when setting it to 1, not entirely sure why
        // can't really do anything else though?
        msaaSamples = vk::SampleCountFlagBits::e1;
      }
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
  const auto indices = findQueueFamilies(physicalDevice, surface.get());
  graphicsQueueFamily = indices.graphicsFamily.value();
  presentQueueFamily = indices.presentFamily.value();

  vk::ArrayProxy<float> queuePriorities = 1.0f;
  std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
  std::set<uint32_t> uniqueFamilies = {graphicsQueueFamily, presentQueueFamily};

  for (uint32_t queueFamily : uniqueFamilies) {
    vk::DeviceQueueCreateInfo queueCreateInfo{};
    queueCreateInfo.setQueueFamilyIndex(queueFamily);
    queueCreateInfo.setQueuePriorities(queuePriorities);
    queueCreateInfos.push_back(queueCreateInfo);
  }

  vk::PhysicalDeviceFeatures supportedFeatures = physicalDevice.getFeatures();
  if (!supportedFeatures.samplerAnisotropy) {
    throw std::runtime_error("Device doesn't support sampler anisotropy");
  }
  if (ENABLE_SAMPLE_SHADING && !supportedFeatures.sampleRateShading) {
    throw std::runtime_error("Device doesn't support sample rate shading");
  }

  vk::PhysicalDeviceFeatures deviceFeatures{};
  deviceFeatures.samplerAnisotropy = VK_TRUE;
  if (ENABLE_SAMPLE_SHADING) {
    deviceFeatures.sampleRateShading = VK_TRUE;
  }

  const auto supportedExtensions =
      physicalDevice.enumerateDeviceExtensionProperties();
  for (const auto &ext : DEVICE_EXTENSIONS) {
    bool found = std::ranges::any_of(supportedExtensions, [&](const auto &e) {
      return strcmp(ext, e.extensionName) == 0;
    });
    if (!found) {
      throw std::runtime_error(std::string("Missing required extension: ") +
                               ext);
    }
  }

  const auto createInfo =
      vk::DeviceCreateInfo()
          .setQueueCreateInfos(queueCreateInfos)
          .setPEnabledFeatures(&deviceFeatures)
          .setPEnabledExtensionNames(DEVICE_EXTENSIONS)
          .setPNext(vk::PhysicalDeviceSynchronization2FeaturesKHR{}
                        .setSynchronization2(vk::True));

  auto device = physicalDevice.createDeviceUnique(createInfo);
  DYNAMIC_DISPATCHER = vk::detail::DispatchLoaderDynamic(
      instance.get(), vkGetInstanceProcAddr, device.get(), vkGetDeviceProcAddr);

  graphicsQueue = device->getQueue(graphicsQueueFamily, 0);
  presentQueue = device->getQueue(presentQueueFamily, 0);

  return device;
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

vk::UniqueCommandPool Engine::createCommandPool() {
  auto info = vk::CommandPoolCreateInfo{}
                  .setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
                  .setQueueFamilyIndex(graphicsQueueFamily);
  return device->createCommandPoolUnique(info);
}
} // namespace Vulking

QueueFamilyIndices findQueueFamilies(vk::PhysicalDevice physicalDevice,
                                     vk::SurfaceKHR surface) {
  QueueFamilyIndices indices;

  const auto queueFamilies = physicalDevice.getQueueFamilyProperties();

  for (const auto [i, queueFamily] :
       std::ranges::views::enumerate(queueFamilies)) {
    if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) {
      indices.graphicsFamily = i;
    }

    const auto presentSupport = physicalDevice.getSurfaceSupportKHR(i, surface);
    if (presentSupport) {
      indices.presentFamily = i;
    }

    if (indices.isComplete()) {
      break;
    }
  }

  return indices;
}
