#pragma once

#include <iostream>
#include <stdexcept>
#include <vulkan/vulkan.hpp>

#define CHK(x, msg)                                                            \
  if (x != VK_SUCCESS) {                                                       \
    throw std::runtime_error(msg);                                             \
  }

#define STYPE(x) VK_STRUCTURE_TYPE_##x

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

#ifndef VULKING_ALLOCATOR
#define VULKING_ALLOCATOR nullptr
#endif

#ifndef ENABLE_SAMPLE_SHADING
#define ENABLE_SAMPLE_SHADING false
#endif

inline static const VkAllocationCallbacks *allocator = VULKING_ALLOCATOR;
