#pragma once
#include "../common.hpp"
#include "Instance.hpp"

namespace Vulking {
class DebugMessenger {
public:
  DebugMessenger(const Instance& instance);

  operator VkDebugUtilsMessengerEXT() const;

  void release();

private:
  const Instance& instance;
  VkDebugUtilsMessengerEXT messenger{};

  void createDebugMessenger();
  static VKAPI_ATTR VkBool32 VKAPI_CALL
  debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                VkDebugUtilsMessageTypeFlagsEXT messageType,
                const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                void *pUserData);
};
} // namespace Vulking
