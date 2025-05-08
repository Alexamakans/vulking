#pragma once

#include "Instance.hpp"
#include <vulkan/vulkan_core.h>

class Surface {
public:
  Surface(const Instance &instance, VkSurfaceKHR surface);
  operator VkSurfaceKHR() const;
  ~Surface();

private:
  Instance instance;
  VkSurfaceKHR surface;
};
