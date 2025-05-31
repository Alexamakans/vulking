#include "Engine.hpp"

#include "Constants.hpp"
#include "Functions.hpp"
#include "UniqueSurface.hpp"

#include <cassert>
#include <iostream>
#include <optional>
#include <ranges>
#include <set>

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
Engine *Engine::engineInstance = nullptr;

Engine::Engine(GLFWwindow *window, const char *applicationInfo,
               uint32_t applicationVersion,
               const std::vector<const char *> &requiredExtensions) {
  Engine::engineInstance = this;

  context.window = window;
  context.instance =
      createInstance(applicationInfo, applicationVersion, requiredExtensions);

  VkSurfaceKHR _surface;
  VkResult result = glfwCreateWindowSurface(
      (VkInstance)getVulkanHandle(context.instance.get()), window, nullptr,
      &_surface);
  if (result != VK_SUCCESS) {
    throw std::runtime_error("failed to create window surface");
  }
  context.surface = std::move(
      UniqueSurface(context.instance.get(), vk::SurfaceKHR(_surface)));

  context.physicalDevice = getSuitablePhysicalDevice();
  context.device = createDevice();

  context.commandPool = createCommandPool();

  // Move swapchain stuff to own function so we can recreate easily
  {
    context.swapchain.handle = createSwapchain();
    context.swapchain.images =
        context.device->getSwapchainImagesKHR(context.swapchain.handle.get());
    const auto count = context.swapchain.images.size();

    context.swapchain.views.resize(count);
    for (uint32_t i = 0; i < count; i++) {
      context.swapchain.views[i] = context.createImageViewUnique(
          context.swapchain.images[i], context.swapchain.imageFormat,
          vk::ImageAspectFlagBits::eColor, 1);
    }

    {
      // create resources

      const auto width = context.swapchain.extent.width;
      const auto height = context.swapchain.extent.height;
      // color image (extract later)
      {
        context.swapchain.color =
            Image(width, height, 1, context.msaaSamples,
                  context.swapchain.imageFormat, vk::ImageTiling::eOptimal,
                  vk::ImageUsageFlagBits::eTransientAttachment |
                      vk::ImageUsageFlagBits::eColorAttachment,
                  vk::MemoryPropertyFlagBits::eDeviceLocal);
        context.swapchain.colorView = context.createImageViewUnique(
            context.swapchain.color.image.get(), context.swapchain.imageFormat,
            vk::ImageAspectFlagBits::eColor, 1);
      }

      // depth image (extract later)
      {
        const auto depthFormat = findDepthFormat();
        context.swapchain.depth =
            Image(width, height, 1, context.msaaSamples, depthFormat,
                  vk::ImageTiling::eOptimal,
                  vk::ImageUsageFlagBits::eTransientAttachment |
                      vk::ImageUsageFlagBits::eDepthStencilAttachment,
                  vk::MemoryPropertyFlagBits::eDeviceLocal);
        context.swapchain.depthView = context.createImageViewUnique(
            context.swapchain.depth.image.get(), depthFormat,
            vk::ImageAspectFlagBits::eDepth, 1);
      }
    }
  }

  // command buffers (extract later)
  {
    context.commandBuffers = context.device->allocateCommandBuffersUnique(
        vk::CommandBufferAllocateInfo()
            .setCommandPool(context.commandPool.get())
            .setLevel(vk::CommandBufferLevel::ePrimary)
            .setCommandBufferCount(context.swapchain.imageCount));
    for (const auto &[i, cmd] :
         std::ranges::views::enumerate(context.commandBuffers)) {
      NAME_OBJECT(context.device, cmd.get(),
                  std::format("engine_command_buffer_{}", i));
    }
  }

  // sync objects (extract later)
  {
    // Create the fence signaled to simplify our sync loop in begin/end render.
    const auto fenceInfo =
        vk::FenceCreateInfo().setFlags(vk::FenceCreateFlagBits::eSignaled);
    const auto count = context.swapchain.imageCount;
    context.inFlightFences.resize(count);

    const auto semaphoreInfo = vk::SemaphoreCreateInfo();
    context.imageAvailableSemaphores.resize(count);
    context.renderFinishedSemaphores.resize(count);

    for (uint32_t i = 0; i < count; i++) {
      context.inFlightFences[i] = context.device->createFenceUnique(fenceInfo);
      context.imageAvailableSemaphores[i] =
          context.device->createSemaphoreUnique(semaphoreInfo);
      context.renderFinishedSemaphores[i] =
          context.device->createSemaphoreUnique(semaphoreInfo);
    }
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
  appInfo.setApiVersion(vk::ApiVersion13);
  vk::InstanceCreateInfo info{};
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
  auto physicalDevices = context.instance->enumeratePhysicalDevices();
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
          context.msaaSamples = sampleCount;
          break;
        }
      }

      if (context.msaaSamples == vk::SampleCountFlagBits{}) {
        // validation gets angry when setting it to 1, not entirely sure why
        // can't really do anything else though?
        context.msaaSamples = vk::SampleCountFlagBits::e1;
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

  LOG("GPU Device properties:");
  LOG("\t vendorID = " << props.vendorID);
  LOG("\t deviceID = " << props.deviceID);
  LOG("\t apiVersion = " << props.apiVersion);
  LOG("\t driverVersion = " << props.driverVersion);
  LOG("\t deviceName = " << props.deviceName);

  return props.deviceType == vk::PhysicalDeviceType::eDiscreteGpu ||
         props.deviceType == vk::PhysicalDeviceType::eIntegratedGpu;
}

vk::UniqueDevice Engine::createDevice() {
  const auto indices =
      findQueueFamilies(context.physicalDevice, context.surface.get());
  context.graphicsQueueFamily = indices.graphicsFamily.value();
  context.presentQueueFamily = indices.presentFamily.value();

  vk::ArrayProxy<float> queuePriorities = 1.0f;
  std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
  std::set<uint32_t> uniqueFamilies = {context.graphicsQueueFamily,
                                       context.presentQueueFamily};

  for (uint32_t queueFamily : uniqueFamilies) {
    vk::DeviceQueueCreateInfo queueCreateInfo{};
    queueCreateInfo.setQueueFamilyIndex(queueFamily);
    queueCreateInfo.setQueuePriorities(queuePriorities);
    queueCreateInfos.push_back(queueCreateInfo);
  }

  vk::PhysicalDeviceFeatures supportedFeatures =
      context.physicalDevice.getFeatures();
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
      context.physicalDevice.enumerateDeviceExtensionProperties();
  for (const auto &ext : DEVICE_EXTENSIONS) {
    bool found = std::ranges::any_of(supportedExtensions, [&](const auto &e) {
      return strcmp(ext, e.extensionName) == 0;
    });
    if (!found) {
      throw std::runtime_error(std::string("Missing required extension: ") +
                               ext);
    }
  }

  const auto sync2Features =
      vk::PhysicalDeviceSynchronization2FeaturesKHR{}.setSynchronization2(
          vk::True);
  const auto createInfo = vk::DeviceCreateInfo()
                              .setQueueCreateInfos(queueCreateInfos)
                              .setPEnabledFeatures(&deviceFeatures)
                              .setPEnabledExtensionNames(DEVICE_EXTENSIONS)
                              .setPNext(&sync2Features);

  auto device = context.physicalDevice.createDeviceUnique(createInfo);
  DYNAMIC_DISPATCHER = vk::detail::DispatchLoaderDynamic(
      context.instance.get(), vkGetInstanceProcAddr, device.get(),
      vkGetDeviceProcAddr);

  context.graphicsQueue = device->getQueue(context.graphicsQueueFamily, 0);
  context.presentQueue = device->getQueue(context.presentQueueFamily, 0);

  return device;
}

vk::UniqueSwapchainKHR Engine::createSwapchain() {
  auto caps = context.physicalDevice.getSurfaceCapabilitiesKHR(context.surface);
  auto formats = context.physicalDevice.getSurfaceFormatsKHR(context.surface);
  auto presentModes =
      context.physicalDevice.getSurfacePresentModesKHR(context.surface);

  auto extent = chooseSwapExtent(caps);
  auto format = chooseSwapSurfaceFormat(formats);
  auto imageCount = caps.minImageCount + 1;
  if (caps.maxImageCount > 0 && imageCount > caps.maxImageCount) {
    imageCount = caps.maxImageCount;
  }
  context.swapchain.imageCount = imageCount;

  vk::SwapchainCreateInfoKHR info{};
  info.setImageFormat(format.format)
      .setImageColorSpace(format.colorSpace)
      .setSurface(context.surface)
      .setPresentMode(chooseSwapPresentMode(presentModes))
      .setMinImageCount(imageCount)
      .setImageExtent(extent)
      .setImageArrayLayers(1)
      .setImageUsage(vk::ImageUsageFlagBits::eColorAttachment);

  if (context.graphicsQueueFamily != context.presentQueueFamily) {
    info.setImageSharingMode(vk::SharingMode::eConcurrent);
    std::array<uint32_t, 2> indices{context.graphicsQueueFamily,
                                    context.presentQueueFamily};
    info.setQueueFamilyIndices(indices);
  } else {
    info.setImageSharingMode(vk::SharingMode::eExclusive);
  }

  info.setPreTransform(caps.currentTransform);
  info.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque);
  info.setClipped(vk::True);
  info.setOldSwapchain(VK_NULL_HANDLE);

  context.swapchain.imageFormat = format.format;
  context.swapchain.extent = extent;

  return context.device->createSwapchainKHRUnique(info);
}

vk::SurfaceFormatKHR Engine::chooseSwapSurfaceFormat(
    const std::vector<vk::SurfaceFormatKHR> &availableFormats) {
  for (const auto &availableFormat : availableFormats) {
    if (availableFormat.format == vk::Format::eB8G8R8A8Srgb &&
        availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
      return availableFormat;
    }
  }

  return availableFormats[0];
}

vk::PresentModeKHR Engine::chooseSwapPresentMode(
    const std::vector<vk::PresentModeKHR> &availablePresentModes) {
  for (const auto &availablePresentMode : availablePresentModes) {
    if (availablePresentMode == vk::PresentModeKHR::eMailbox) {
      return availablePresentMode;
    }
  }

  return vk::PresentModeKHR::eFifo;
}

vk::Extent2D
Engine::chooseSwapExtent(const vk::SurfaceCapabilitiesKHR &capabilities) {
  if (capabilities.currentExtent.width !=
      std::numeric_limits<uint32_t>::max()) {
    return capabilities.currentExtent;
  } else {
    int width, height;
    glfwGetFramebufferSize(context.window, &width, &height);

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
                  .setQueueFamilyIndex(context.graphicsQueueFamily);
  return context.device->createCommandPoolUnique(info);
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
