#pragma once
#include "../common.hpp"
#include <vector>
#include <vulkan/vulkan_core.h>

namespace Vulking {
class Instance {
public:
  Instance(const std::vector<const char *> &requiredExtensions,
           bool enableValidation = enableValidationLayers);
  void release() { vkDestroyInstance(instance, allocator); }
  operator VkInstance() const;

private:
  VkInstance instance{};
  bool validationEnabled{};

  VkInstance
  createInstance(const std::vector<const char *> &requiredExtensions);
  bool checkValidationLayerSupport() const;
};
} // namespace Vulking
