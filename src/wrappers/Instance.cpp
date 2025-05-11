#include "Instance.hpp"
#include <GLFW/glfw3.h>

const std::vector<const char *> validationLayers = {
    "VK_LAYER_KHRONOS_validation"};

Vulking::Instance::Instance(const std::vector<const char *> &requiredExtensions,
                            bool enableValidation)
    : instance(createInstance(requiredExtensions)),
      validationEnabled(enableValidation) {
  if (validationEnabled && !checkValidationLayerSupport()) {
    throw std::runtime_error("Validation layers requested, but not available.");
  }

  pfnSetDebugUtilsObjectNameEXT =
      reinterpret_cast<PFN_vkSetDebugUtilsObjectNameEXT>(
          vkGetInstanceProcAddr(instance, "vkSetDebugUtilsObjectNameEXT"));
}

Vulking::Instance::operator VkInstance() const { return instance; }

VkInstance Vulking::Instance::createInstance(
    const std::vector<const char *> &requiredExtensions) {
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

  createInfo.enabledExtensionCount =
      static_cast<uint32_t>(requiredExtensions.size());
  createInfo.ppEnabledExtensionNames = requiredExtensions.data();

  VkInstance vkInstance;
  CHK(vkCreateInstance(&createInfo, allocator, &vkInstance),
      "failed to create vulkan instance.");
  return vkInstance;
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
