#include "UniqueSurface.hpp"

Vulking::UniqueSurface::UniqueSurface() : instance(nullptr), surface(nullptr) {}

Vulking::UniqueSurface::UniqueSurface(vk::Instance instance,
                                      vk::SurfaceKHR surface)
    : instance(instance), surface(surface) {}

Vulking::UniqueSurface::~UniqueSurface() { reset(); }

vk::SurfaceKHR Vulking::UniqueSurface::get() const { return surface; }
