#include <cstring>
#include <iomanip>
#include <vector>

#include <vulkan/vulkan_core.h>

#include "common.hpp"
#include "debug_messenger.hpp"

static VKAPI_ATTR VkBool32 VKAPI_CALL
debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
              VkDebugUtilsMessageTypeFlagsEXT messageType,
              const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
              void *pUserData) {
  if (messageSeverity >= VULKING_DEBUG_VALIDATION_SEVERITY) {
    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
  }
  return VK_FALSE;
}

DebugMessenger::DebugMessenger(VkInstance instance,
                               const VkAllocationCallbacks *allocator)
    : instance(instance), allocator(allocator) {
  VkDebugUtilsMessengerCreateInfoEXT createInfo{};
  createInfo.sType = STYPE(DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT);
  createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
  createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
  createInfo.pfnUserCallback = debugCallback;
  createInfo.pUserData = nullptr; // Optional

  auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
      instance, "vkCreateDebugUtilsMessengerEXT");
  if (func != nullptr) {
    CHK(func(instance, &createInfo, allocator, &vkDebugMessenger),
        "failed to set up debug messenger")
  } else {
    throw std::runtime_error("failed to find vkCreateDebugUtilsMessengerEXT");
  }
}

DebugMessenger::~DebugMessenger() {
  auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
      instance, "vkDestroyDebugUtilsMessengerEXT");
  if (func != nullptr) {
    func(instance, vkDebugMessenger, allocator);
  }
}

bool DebugMessenger::checkValidationLayerSupport() {
  uint32_t layerCount;
  CHK(vkEnumerateInstanceLayerProperties(&layerCount, nullptr),
      "failed getting layer count")

  std::vector<VkLayerProperties> availableLayers(layerCount);
  CHK(vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data()),
      "failed getting layer properties")

  for (const char *layerName : validationLayers) {
    bool layerFound = false;

    for (const auto &layerProperties : availableLayers) {
      if (strcmp(layerName, layerProperties.layerName) == 0) {
        layerFound = true;
        break;
      }
    }

    if (!layerFound) {
      std::cerr << "layer " << std::quoted(layerName) << " not found"
                << std::endl;
      return false;
    }
  }

  return true;
}
