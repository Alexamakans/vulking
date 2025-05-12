#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_handles.hpp>

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>

inline const char *vkResultToString(VkResult result) {
  switch (result) {
  case VK_SUCCESS:
    return "VK_SUCCESS";
  case VK_NOT_READY:
    return "VK_NOT_READY";
  case VK_TIMEOUT:
    return "VK_TIMEOUT";
  case VK_EVENT_SET:
    return "VK_EVENT_SET";
  case VK_EVENT_RESET:
    return "VK_EVENT_RESET";
  case VK_INCOMPLETE:
    return "VK_INCOMPLETE";
  case VK_ERROR_OUT_OF_HOST_MEMORY:
    return "VK_ERROR_OUT_OF_HOST_MEMORY";
  case VK_ERROR_OUT_OF_DEVICE_MEMORY:
    return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
  case VK_ERROR_INITIALIZATION_FAILED:
    return "VK_ERROR_INITIALIZATION_FAILED";
  case VK_ERROR_DEVICE_LOST:
    return "VK_ERROR_DEVICE_LOST";
  case VK_ERROR_MEMORY_MAP_FAILED:
    return "VK_ERROR_MEMORY_MAP_FAILED";
  case VK_ERROR_LAYER_NOT_PRESENT:
    return "VK_ERROR_LAYER_NOT_PRESENT";
  case VK_ERROR_EXTENSION_NOT_PRESENT:
    return "VK_ERROR_EXTENSION_NOT_PRESENT";
  case VK_ERROR_FEATURE_NOT_PRESENT:
    return "VK_ERROR_FEATURE_NOT_PRESENT";
  case VK_ERROR_INCOMPATIBLE_DRIVER:
    return "VK_ERROR_INCOMPATIBLE_DRIVER";
  case VK_ERROR_TOO_MANY_OBJECTS:
    return "VK_ERROR_TOO_MANY_OBJECTS";
  case VK_ERROR_FORMAT_NOT_SUPPORTED:
    return "VK_ERROR_FORMAT_NOT_SUPPORTED";
  case VK_ERROR_SURFACE_LOST_KHR:
    return "VK_ERROR_SURFACE_LOST_KHR";
  case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
    return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
  case VK_ERROR_OUT_OF_POOL_MEMORY:
    return "VK_ERROR_OUT_OF_POOL_MEMORY";
  // Add more as needed
  default:
    std::cout << "result: " << result << std::endl;
    return "Unknown VkResult";
  }
}

#define CHK(expr, msg)                                                         \
  do {                                                                         \
    VkResult _vk_result = (expr);                                              \
    if (_vk_result != VK_SUCCESS) {                                            \
      throw std::runtime_error(std::format("{}:{} >{} (VkResult: {})",         \
                                           __FILE__, __LINE__, msg,            \
                                           vkResultToString(_vk_result)));     \
    }                                                                          \
  } while (0)

#define STYPE(x) VK_STRUCTURE_TYPE_##x

#ifdef NDEBUG
constexpr bool enableValidationLayers = false;
#define NAME_OBJECT(...)
#else
constexpr bool enableValidationLayers = true;

inline PFN_vkSetDebugUtilsObjectNameEXT pfnSetDebugUtilsObjectNameEXT = nullptr;

#define NAME_OBJECT(device, type, handle, name)                                \
  nameObject(device, type, handle, name)

inline void _nameObject(VkDevice device, VkObjectType objectType,
                        uint64_t handle, const char *name) {
  VkDebugUtilsObjectNameInfoEXT nameInfo = {
      .sType = STYPE(DEBUG_UTILS_OBJECT_NAME_INFO_EXT),
      .objectType = objectType,
      .objectHandle = handle,
      .pObjectName = name,
  };

  if (pfnSetDebugUtilsObjectNameEXT) {
    CHK(pfnSetDebugUtilsObjectNameEXT(device, &nameInfo),
        "failed to name object");
  }
}

template <typename T>
inline void nameObject(VkDevice device, VkObjectType objectType, T handle,
                       const char *name) {
  _nameObject(device, objectType, (uint64_t)(uintptr_t)handle, name);
}

template <typename T>
inline void nameObject(VkDevice device, VkObjectType objectType, T handle,
                       const std::string &name) {
  _nameObject(device, objectType, (uint64_t)(uintptr_t)handle, name.c_str());
}

#endif

#ifndef VULKING_ALLOCATOR
#define VULKING_ALLOCATOR nullptr
#endif

#ifndef ENABLE_SAMPLE_SHADING
#define ENABLE_SAMPLE_SHADING false
#endif

inline static const VkAllocationCallbacks *ALLOCATOR = VULKING_ALLOCATOR;

static std::vector<char> readFile(const std::string &filename) {
  std::ifstream file(filename, std::ios::ate | std::ios::binary);

  if (!file.is_open()) {
    throw std::runtime_error(std::format("failed to open file '{}'", filename));
  }

  size_t fileSize = (size_t)file.tellg();
  std::vector<char> buffer(fileSize);

  file.seekg(0);
  file.read(buffer.data(), fileSize);

  file.close();

  return buffer;
}
