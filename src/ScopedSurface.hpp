#pragma once

#include "Common.hpp"

namespace Vulking {
class ScopedSurface {
public:
  ScopedSurface();
  ScopedSurface(vk::Instance instance, vk::SurfaceKHR surface);
  ~ScopedSurface();

  operator vk::SurfaceKHR() const { return m_surface; }

  vk::SurfaceKHR get() const;

private:
  vk::Instance m_instance;
  vk::SurfaceKHR m_surface;
};
} // namespace Vulking
