#pragma once
#include "../common.hpp"

namespace Vulking {
class DebugMessenger {
public:
  DebugMessenger(VkInstance instance);
  ~DebugMessenger();

  operator VkDebugUtilsMessengerEXT() const;

private:
  VkInstance instance;
  VkDebugUtilsMessengerEXT messenger{};

  void createDebugMessenger();
  static VKAPI_ATTR VkBool32 VKAPI_CALL
  debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                VkDebugUtilsMessageTypeFlagsEXT messageType,
                const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                void *pUserData);
};
} // namespace Vulking
