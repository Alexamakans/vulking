#pragma once

#include "Common.hpp"
#include <vulkan/vulkan_core.h>

#ifdef NDEBUG
inline const bool ENABLE_VALIDATION_LAYERS = true;
#else
inline const bool ENABLE_VALIDATION_LAYERS = false;
inline const std::vector<const char *> VALIDATION_LAYERS = {
    "VK_LAYER_KHRONOS_validation"};
#endif

inline const std::vector<const char *> DEVICE_EXTENSIONS = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME,
    VK_KHR_COPY_COMMANDS_2_EXTENSION_NAME};
