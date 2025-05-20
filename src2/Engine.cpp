#include "Engine.hpp"
#include "Common.hpp"
#include "Constants.hpp"

#include <cassert>
#include <iostream>
#include <set>

namespace Vulking {

vk::UniqueInstance Engine::instance;
vk::PhysicalDevice Engine::physicalDevice;
vk::UniqueDevice Engine::device;
vk::CommandPool Engine::commandPool;
vk::DescriptorPool Engine::descriptorPool;

Engine::Engine(GLFWwindow *window, const char *applicationInfo,
               uint32_t applicationVersion,
               const std::vector<const char *> &requiredExtensions) {
  Engine::instance =
      createInstance(applicationInfo, applicationVersion, requiredExtensions);

  VkSurfaceKHR _surface;
  VkResult result = glfwCreateWindowSurface(
      (VkInstance)getVulkanHandle(instance.get()), window, nullptr, &_surface);
  if (result != VK_SUCCESS) {
    throw std::runtime_error("failed to create window surface");
  }
  surface = vk::SurfaceKHR(_surface);

  Engine::physicalDevice = getSuitablePhysicalDevice();
  Engine::device = createDevice();
  DYNAMIC_DISPATCHER = vk::detail::DispatchLoaderDynamic(
      instance.get(), vkGetInstanceProcAddr, device.get(), vkGetDeviceProcAddr);
}

Engine::~Engine() {
  device.release().destroy();
  instance.release().destroy(ALLOCATOR);
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

vk::UniqueInstance
Engine::createInstance(const char *applicationInfo, uint32_t applicationVersion,
                       const std::vector<const char *> &requiredExtensions) {
  vk::ApplicationInfo appInfo{};
  appInfo.setPApplicationName(applicationInfo);
  appInfo.setApplicationVersion(applicationVersion);
  appInfo.setPEngineName("Vulking");
  appInfo.setEngineVersion(VK_MAKE_VERSION(0, 0, 1));
  appInfo.setApiVersion(VK_API_VERSION_1_3);
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
} // namespace Vulking
