#include "Surface.hpp"
#include "../common.hpp"

Vulking::Surface::Surface(const Instance &instance, VkSurfaceKHR surface)
    : instance(instance), surface(surface) {}

Vulking::Surface::operator VkSurfaceKHR() const { return surface; }
