#include "DebugMessenger.hpp"

Vulking::DebugMessenger::DebugMessenger(const Instance &instance)
    : instance(instance) {
  if (enableValidationLayers) {
    createDebugMessenger();
  }
}

void Vulking::DebugMessenger::release() {
  if (enableValidationLayers && messenger) {
    auto destroyFn = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
        vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));
    if (destroyFn) {
      destroyFn(instance, messenger, allocator);
    }
  }
}

Vulking::DebugMessenger::operator VkDebugUtilsMessengerEXT() const {
  return messenger;
}

void Vulking::DebugMessenger::createDebugMessenger() {
  VkDebugUtilsMessengerCreateInfoEXT createInfo{};
  createInfo.sType = STYPE(DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT);
  createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

  createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

  createInfo.pfnUserCallback = debugCallback;
  createInfo.pUserData = nullptr;

  auto createFn = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
      vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));

  if (!createFn) {
    throw std::runtime_error("Could not load vkCreateDebugUtilsMessengerEXT");
  }

  CHK(createFn(instance, &createInfo, allocator, &messenger),
      "failed to set up debug messenger.");
}

VKAPI_ATTR VkBool32 VKAPI_CALL Vulking::DebugMessenger::debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT severity,
    VkDebugUtilsMessageTypeFlagsEXT,
    const VkDebugUtilsMessengerCallbackDataEXT *callbackData, void *) {
  if (severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
    std::cerr << "[Vulkan] " << callbackData->pMessage << std::endl;
  }
  return VK_FALSE;
}
