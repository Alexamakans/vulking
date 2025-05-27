#include "ScopedSurface.hpp"

Vulking::ScopedSurface::ScopedSurface() {}

Vulking::ScopedSurface::ScopedSurface(vk::Instance instance,
                                      vk::SurfaceKHR surface)
    : m_instance(instance), m_surface(surface) {}

Vulking::ScopedSurface::~ScopedSurface() {
  if (m_surface) {
    m_instance.destroySurfaceKHR(m_surface);
  }
}

vk::SurfaceKHR Vulking::ScopedSurface::get() const { return m_surface; }
