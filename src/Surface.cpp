#include "Surface.hpp"
#include "common.hpp"

Surface::Surface(const Instance &instance, VkSurfaceKHR surface)
    : instance(instance), surface(surface) {}

Surface::operator VkSurfaceKHR() const { return surface; }

Surface::~Surface() { vkDestroySurfaceKHR(instance, surface, allocator); }
