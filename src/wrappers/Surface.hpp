#pragma once

#include "Instance.hpp"
#include <vulkan/vulkan_core.h>

namespace Vulking {
class Surface {
public:
  Surface(const Instance &instance, VkSurfaceKHR surface);
  void release() { vkDestroySurfaceKHR(instance, surface, allocator); };
  operator VkSurfaceKHR() const;

private:
  const Instance &instance;
  VkSurfaceKHR surface{};
};
} // namespace Vulking
