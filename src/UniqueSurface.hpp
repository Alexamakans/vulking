#pragma once

#include "Common.hpp"

namespace Vulking {
class UniqueSurface {
public:
  UniqueSurface(const UniqueSurface &) = delete;
  UniqueSurface &operator=(const UniqueSurface &) = delete;
  UniqueSurface(UniqueSurface &&other) noexcept
      : instance(other.instance), surface(other.surface) {
    other.instance = nullptr;
    other.surface = nullptr;
  };
  UniqueSurface &operator=(UniqueSurface &&other) noexcept {
    if (this != &other) {
      reset();
      instance = other.instance;
      surface = other.surface;
      other.instance = nullptr;
      other.surface = nullptr;
    }
    return *this;
  };

  UniqueSurface();
  UniqueSurface(vk::Instance instance, vk::SurfaceKHR surface);
  ~UniqueSurface();

  operator vk::SurfaceKHR() const { return surface; }

  vk::SurfaceKHR get() const;
  void reset() {
    if (surface) {
      assert(instance);
      instance.destroySurfaceKHR(surface);
      surface = nullptr;
    }
    instance = nullptr;
  }

private:
  vk::Instance instance;
  vk::SurfaceKHR surface;
};
} // namespace Vulking
