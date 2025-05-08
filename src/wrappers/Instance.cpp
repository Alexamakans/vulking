#include "Instance.hpp"

const std::vector<const char *> validationLayers = {
    "VK_LAYER_KHRONOS_validation"};

Vulking::Instance::Instance(bool enableValidation)
    : validationEnabled(enableValidation) {
  if (validationEnabled && !checkValidationLayerSupport()) {
    throw std::runtime_error("Validation layers requested, but not available.");
  }
  createInstance();
}

Vulking::Instance::~Instance() {
  if (instance) {
    vkDestroyInstance(instance, allocator);
  }
}

Vulking::Instance::operator VkInstance() const { return instance; }

void Vulking::Instance::createInstance() {
  VkApplicationInfo appInfo{};
  appInfo.sType = STYPE(APPLICATION_INFO);
  appInfo.pApplicationName = "Vulkan App";
  appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.pEngineName = "No Engine";
  appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.apiVersion = VK_API_VERSION_1_3;

  VkInstanceCreateInfo createInfo{};
  createInfo.sType = STYPE(INSTANCE_CREATE_INFO);
  createInfo.pApplicationInfo = &appInfo;

  auto extensions = getRequiredExtensions();
  createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
  createInfo.ppEnabledExtensionNames = extensions.data();

  if (validationEnabled) {
    createInfo.enabledLayerCount =
        static_cast<uint32_t>(validationLayers.size());
    createInfo.ppEnabledLayerNames = validationLayers.data();
  } else {
    createInfo.enabledLayerCount = 0;
    createInfo.ppEnabledLayerNames = nullptr;
  }

  CHK(vkCreateInstance(&createInfo, allocator, &instance),
      "Failed to create Vulkan instance.");
}

std::vector<const char *> Vulking::Instance::getRequiredExtensions() const {
  std::vector<const char *> extensions = {
      "VK_KHR_surface" // Platform-specific surface extensions should be
                       // appended elsewhere
  };

#ifndef NDEBUG
  extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

  return extensions;
}

bool Vulking::Instance::checkValidationLayerSupport() const {
  uint32_t layerCount;
  vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

  std::vector<VkLayerProperties> availableLayers(layerCount);
  vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

  for (const char *layerName : validationLayers) {
    bool found = false;
    for (const auto &layer : availableLayers) {
      if (strcmp(layerName, layer.layerName) == 0) {
        found = true;
        break;
      }
    }
    if (!found)
      return false;
  }
  return true;
}
