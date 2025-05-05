#pragma once

#include <vulkan/vulkan_core.h>

#ifndef VULKING_DEBUG_VALIDATION_SEVERITY
#define VULKING_DEBUG_VALIDATION_SEVERITY                                      \
  VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT
#endif

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData);

class DebugMessenger {
public:
  DebugMessenger() {};
  DebugMessenger(VkInstance instance, const VkAllocationCallbacks *allocator);
  ~DebugMessenger();

private:
  bool checkValidationLayerSupport();

  VkDebugUtilsMessengerEXT vkDebugMessenger;

  VkInstance instance;
  const VkAllocationCallbacks *allocator;
};
