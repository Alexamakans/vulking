#include "Engine.hpp"

#include "Device.hpp"

#include <cassert>
#include <iostream>
#include <vulkan/vulkan_core.h>

Vulking::Engine::Engine(GLFWwindow *window, const char *applicationInfo,
                        uint32_t applicationVersion,
                        std::vector<const char *> &requiredExtensions) {
  instance =
      createInstance(applicationInfo, applicationVersion, requiredExtensions);
  CHK(glfwCreateWindowSurface(instance, window, ALLOCATOR, &surface),
      "failed to create surface");
  physicalDevice = PhysicalDevice(getSuitablePhysicalDevice(instance), surface);
  device = Device(physicalDevice);
}

VkInstance
Vulking::Engine::createInstance(const char *applicationInfo,
                                uint32_t applicationVersion,
                                std::vector<const char *> &requiredExtensions) {
  VkApplicationInfo appInfo{};
  appInfo.sType = STYPE(APPLICATION_INFO);
  appInfo.pApplicationName = applicationInfo;
  appInfo.applicationVersion = applicationVersion;
  appInfo.pEngineName = "Vulking";
  appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 1);
  appInfo.apiVersion = VK_API_VERSION_1_3;
  VkInstanceCreateInfo info{};
  info.sType = STYPE(INSTANCE_CREATE_INFO);
  info.pApplicationInfo = &appInfo;
  info.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size());
  info.ppEnabledExtensionNames = requiredExtensions.data();
  VkInstance _instance;
  CHK(vkCreateInstance(&info, ALLOCATOR, &_instance),
      "failed to create instance");
  return _instance;
}

Vulking::Engine::~Engine() {
  device.destroy();
  vkDestroyInstance(instance, ALLOCATOR);
}

VkSamplerCreateInfo &Vulking::Engine::defaultSamplerInfo() {
  static VkSamplerCreateInfo info{
      .sType = STYPE(SAMPLER_CREATE_INFO),
      .pNext = nullptr,
      .flags = 0,
      .magFilter = VK_FILTER_LINEAR,
      .minFilter = VK_FILTER_LINEAR,
      .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
      .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
      .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
      .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
      .mipLodBias = 0.0f,
      .anisotropyEnable = VK_TRUE,
      .maxAnisotropy =
          physicalDevice.getProperties().limits.maxSamplerAnisotropy,
      .compareEnable = VK_FALSE,
      .compareOp = VK_COMPARE_OP_ALWAYS,
      .minLod = 0.0f,
      .maxLod = std::numeric_limits<float>::max(),
      .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
      .unnormalizedCoordinates = VK_FALSE,
  };

  return info;
}

Sampler Vulking::Engine::createSampler(const VkSamplerCreateInfo &info) {
  Sampler sampler{};
  CHK(vkCreateSampler(device, &info, ALLOCATOR, &sampler),
      "failed to create sampler");
  return sampler;
}

Vulking::Buffer Vulking::Engine::createBuffer(void *src, VkDeviceSize size,
                                              VkBufferUsageFlags usage,
                                              VkMemoryPropertyFlags properties,
                                              const char *name) {
  return Buffer(device, src, size, usage, properties, name);
}

VkPhysicalDevice
Vulking::Engine::getSuitablePhysicalDevice(VkInstance instance) {
  assert(instance != VK_NULL_HANDLE);

  uint32_t deviceCount = 0;
  vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
  if (deviceCount == 0) {
    throw std::runtime_error("failed to find GPUs with Vulkan support.");
  }

  std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
  vkEnumeratePhysicalDevices(instance, &deviceCount, physicalDevices.data());

  for (const auto &physicalDevice : physicalDevices) {
    if (isDeviceSuitable(physicalDevice)) {
      return physicalDevice;
    }
  }

  throw std::runtime_error("failed to find a suitable GPU.");
}

bool Vulking::Engine::isDeviceSuitable(VkPhysicalDevice physicalDevice) const {
  assert(physicalDevice != VK_NULL_HANDLE);
  // This should be extended based on required features, extensions, and queue
  // families.
  VkPhysicalDeviceProperties props;
  vkGetPhysicalDeviceProperties(physicalDevice, &props);

  std::cout << "GPU Device properties:\n";
  std::cout << "\t vendorID = " << props.vendorID << "\n";
  std::cout << "\t deviceType = " << props.deviceType << "\n";
  std::cout << "\t deviceID = " << props.deviceID << "\n";
  std::cout << "\t apiVersion = " << props.apiVersion << "\n";
  std::cout << "\t driverVersion = " << props.driverVersion << "\n";
  std::cout << "\t deviceName = " << props.deviceName << std::endl;

  return props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU ||
         props.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU;
}
