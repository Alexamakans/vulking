#include "Surface.hpp"
#include "../common.hpp"

Vulking::Surface::Surface(Instance instance, VkSurfaceKHR surface)
    : instance(instance), surface(surface) {}

Vulking::Surface::operator VkSurfaceKHR() const { return surface; }

Vulking::Surface::~Surface() {
  vkDestroySurfaceKHR(instance, surface, allocator);
}
