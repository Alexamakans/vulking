#pragma once

#include <vector>
#include <vulkan/vulkan_core.h>

#ifndef VULKING_DEBUG_VALIDATION_SEVERITY
#define VULKING_DEBUG_VALIDATION_SEVERITY                                      \
  VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT
#endif

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData);

const std::vector<const char *> validationLayers = {
    "VK_LAYER_KHRONOS_validation"};

class DebugMessenger {
public:
  DebugMessenger() = default;
  DebugMessenger(VkInstance instance, const VkAllocationCallbacks *allocator);
  ~DebugMessenger();

  bool checkValidationLayerSupport();

private:
  VkDebugUtilsMessengerEXT vkDebugMessenger;

  VkInstance instance;
  const VkAllocationCallbacks *allocator;
};
