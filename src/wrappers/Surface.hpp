#pragma once

#include "Instance.hpp"
#include <vulkan/vulkan_core.h>

namespace Vulking {
class Surface {
public:
  Surface(Instance instance, VkSurfaceKHR surface);
  operator VkSurfaceKHR() const;
  ~Surface();

private:
  Instance instance;
  VkSurfaceKHR surface;
};
} // namespace Vulking
