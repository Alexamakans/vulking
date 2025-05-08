#pragma once

#include "Instance.hpp"
#include <vulkan/vulkan_core.h>

namespace Vulking {
class Surface {
public:
  Surface() = default;
  Surface(Instance instance, VkSurfaceKHR surface);
  Surface(Surface&) = default;
  operator VkSurfaceKHR() const;
  ~Surface();

private:
  Instance instance;
  VkSurfaceKHR surface{};
};
} // namespace Vulking
