#pragma once

#include "Instance.hpp"
#include <vulkan/vulkan_core.h>

namespace Vulking {
class Surface {
public:
  Surface() = default;
  Surface(Instance instance, VkSurfaceKHR surface);
  Surface(Surface &) = default;
  Surface &operator=(const Surface &other) {
    instance = other.instance;
    surface = other.surface;
    return *this;
  }
  operator VkSurfaceKHR() const;
  ~Surface();

private:
  Instance instance;
  VkSurfaceKHR surface{};
};
} // namespace Vulking
