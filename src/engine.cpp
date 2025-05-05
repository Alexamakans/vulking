#include <set>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <map>
#include <memory>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_funcs.hpp>

#include "common.hpp"
#include "engine.hpp"
#include "graphics/gpu.hpp"

#ifndef ENABLE_SAMPLE_SHADING
#define ENABLE_SAMPLE_SHADING false
#endif

Engine::Engine(GLFWwindow *window) { Engine::window = window; };

void Engine::init() {
  createInstance();
  if (enableValidationLayers) {
    new (&debugMessenger) DebugMessenger(Engine::instance, allocator);
  }
  createSurface();
  pickGPU();
  createSwapchain();
}

Engine::~Engine() {
  std::destroy_at(&Engine::gpu);
  if (enableValidationLayers) {
    std::destroy_at(&debugMessenger);
  }
  vkDestroySurfaceKHR(instance, surface, allocator);
  vkDestroyInstance(instance, allocator);
  glfwDestroyWindow(window);
  glfwTerminate();
};

void Engine::createInstance() {
  if (enableValidationLayers && !debugMessenger.checkValidationLayerSupport()) {
    throw std::runtime_error("validation layers requested, but not available");
  }

  VkApplicationInfo appInfo{};
  appInfo.sType = STYPE(APPLICATION_INFO);
  appInfo.pApplicationName = "VulkingApp";
  appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
  appInfo.pEngineName = "Vulking";
  appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 1);
  appInfo.apiVersion = VK_API_VERSION_1_3;

  VkInstanceCreateInfo createInfo{};
  createInfo.sType = STYPE(INSTANCE_CREATE_INFO);
  createInfo.pApplicationInfo = &appInfo;

  auto extensions = getRequiredExtensions();

  createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
  createInfo.ppEnabledExtensionNames = extensions.data();

  std::cout << "required extensions:\n";
  for (const auto &extension : extensions) {
    std::cout << '\t' << extension << std::endl;
  }

  if (enableValidationLayers) {
    createInfo.enabledLayerCount =
        static_cast<uint32_t>(validationLayers.size());
    createInfo.ppEnabledLayerNames = validationLayers.data();
  } else {
    createInfo.enabledLayerCount = 0;
  }

  CHK(vkCreateInstance(&createInfo, nullptr, &instance),
      "failed to create instance")
}

std::vector<const char *> Engine::getRequiredExtensions() {
  uint32_t glfwExtensionCount = 0;
  const char **glfwExtensions;
  glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

  std::vector<const char *> extensions(glfwExtensions,
                                       glfwExtensions + glfwExtensionCount);

  if (enableValidationLayers) {
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  }

  return extensions;
}

void Engine::createSurface() {
  CHK(glfwCreateWindowSurface(Engine::instance, Engine::window, allocator,
                              &Engine::surface),
      "failed to create window surface")
}

void Engine::pickGPU() {
  uint32_t deviceCount = 0;
  CHK(vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr),
      "failed to get physical device count")

  if (deviceCount == 0) {
    throw std::runtime_error("failed to find GPUs with Vulkan support");
  }

  std::vector<VkPhysicalDevice> devices(deviceCount);
  CHK(vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data()),
      "failed to enumerate physical devices")

  std::multimap<int, GPU> candidates;
  for (const auto &device : devices) {
    GPU gpu = GPU(device);
    int score = gpu.rateSuitability();
    candidates.insert(std::make_pair(score, device));
  }

  if (candidates.rbegin()->first > 0) {
    new (&Engine::gpu) GPU(candidates.rbegin()->second);
  } else {
    throw std::runtime_error("failed to find a suitable GPU");
  }
}

void Engine::createDevice() {
  GPU::QueueFamilyIndices indices = Engine::gpu.queueFamilyIndices;
  std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
  std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(),
                                            indices.presentFamily.value()};

  float queuePriority = 1.0f;
  for (uint32_t queueFamily : uniqueQueueFamilies) {
    VkDeviceQueueCreateInfo queueCreateInfo{};
    queueCreateInfo.sType = STYPE(DEVICE_QUEUE_CREATE_INFO);
    queueCreateInfo.queueFamilyIndex = indices.graphicsFamily.value();
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;

    queueCreateInfos.push_back(queueCreateInfo);
  }

  VkPhysicalDeviceFeatures deviceFeatures{
      .sampleRateShading = ENABLE_SAMPLE_SHADING ? VK_TRUE : VK_FALSE,
      .samplerAnisotropy = VK_TRUE,
  };

  VkDeviceCreateInfo createInfo{};
  createInfo.sType = STYPE(DEVICE_CREATE_INFO);
  createInfo.pQueueCreateInfos = queueCreateInfos.data();
  createInfo.queueCreateInfoCount =
      static_cast<uint32_t>(queueCreateInfos.size());

  createInfo.pEnabledFeatures = &deviceFeatures;

  createInfo.enabledExtensionCount =
      static_cast<uint32_t>(deviceExtensions.size());
  createInfo.ppEnabledExtensionNames = deviceExtensions.data();

  if (enableValidationLayers) {
    createInfo.enabledLayerCount =
        static_cast<uint32_t>(validationLayers.size());
    createInfo.ppEnabledLayerNames = validationLayers.data();
  } else {
    createInfo.enabledLayerCount = 0;
  }

  CHK(vkCreateDevice(Engine::gpu.device, &createInfo, nullptr, &device),
      "failed to create logical device")

  vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0,
                   &Engine::graphicsQueue);
  vkGetDeviceQueue(device, indices.presentFamily.value(), 0,
                   &Engine::presentQueue);
}

void Engine::createSwapchain() {
}
