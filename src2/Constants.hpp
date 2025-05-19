#pragma once

#include "Common.hpp"
#include <vector>
#include <vulkan/vulkan.hpp>

#ifdef NDEBUG
inline const bool ENABLE_VALIDATION_LAYERS = true;
#else
inline const bool ENABLE_VALIDATION_LAYERS = false;
inline const std::vector<const char *> VALIDATION_LAYERS = {
    "VK_LAYER_KHRONOS_validation"};
#endif

inline const vk::ArrayProxyNoTemporaries<const char *const> DEVICE_EXTENSIONS =
    {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
