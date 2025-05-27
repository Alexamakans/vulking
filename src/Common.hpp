#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#define VULKAN_HPP_NO_CONSTRUCTORS
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_handles.hpp>
#include <vulkan/vulkan_to_string.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>

template <typename T> uint64_t getVulkanHandle(T const &cppHandle) {
  return uint64_t(static_cast<T::CType>(cppHandle));
}

static const char *vkResultToString(vk::Result result) {
  switch (result) {
  case vk::Result::eSuccess:
    return "Success";
  case vk::Result::eNotReady:
    return "NotReady";
  case vk::Result::eTimeout:
    return "Timeout";
  case vk::Result::eEventSet:
    return "EventSet";
  case vk::Result::eEventReset:
    return "EventReset";
  case vk::Result::eIncomplete:
    return "Incomplete";
  case vk::Result::eErrorOutOfHostMemory:
    return "ErrorOutOfHostMemory";
  case vk::Result::eErrorOutOfDeviceMemory:
    return "ErrorOutOfDeviceMemory";
  case vk::Result::eErrorInitializationFailed:
    return "ErrorInitializationFailed";
  case vk::Result::eErrorDeviceLost:
    return "ErrorDeviceLost";
  case vk::Result::eErrorMemoryMapFailed:
    return "ErrorMemoryMapFailed";
  case vk::Result::eErrorLayerNotPresent:
    return "ErrorLayerNotPresent";
  case vk::Result::eErrorExtensionNotPresent:
    return "ErrorExtensionNotPresent";
  case vk::Result::eErrorFeatureNotPresent:
    return "ErrorFeatureNotPresent";
  case vk::Result::eErrorIncompatibleDriver:
    return "ErrorIncompatibleDriver";
  case vk::Result::eErrorTooManyObjects:
    return "ErrorTooManyObjects";
  case vk::Result::eErrorFormatNotSupported:
    return "ErrorFormatNotSupported";
  case vk::Result::eErrorSurfaceLostKHR:
    return "ErrorSurfaceLostKHR";
  case vk::Result::eErrorNativeWindowInUseKHR:
    return "ErrorNativeWindowInUseKHR";
  case vk::Result::eErrorOutOfPoolMemory:
    return "ErrorOutOfPoolMemory";
  // Add more as needed
  default:
    std::cout << "result: " << result << std::endl;
    return "UnknownError";
  }
}

#define CHK(expr, msg)                                                         \
  do {                                                                         \
    vk::Result __result = (expr);                                              \
    vk::detail::resultCheck(                                                   \
        __result,                                                              \
        std::format("{}:{} [{}] >{} (vk::Result: {})", __FILE__, __LINE__,     \
                    __PRETTY_FUNCTION__, msg, vkResultToString(__result))      \
            .c_str());                                                         \
  } while (0)

#define STYPE(x) VK_STRUCTURE_TYPE_##x

#ifdef NDEBUG
constexpr bool enableValidationLayers = false;
#define NAME_OBJECT(...)
#else
constexpr bool enableValidationLayers = true;

// Initialized by Engine in constructor
inline vk::detail::DispatchLoaderDynamic DYNAMIC_DISPATCHER;

#define NAME_OBJECT(device, handle, name) nameObject(device, handle, name)

inline void _nameObject(vk::UniqueDevice &device, vk::ObjectType objectType,
                        uint64_t handle, const char *name) {
  vk::DebugUtilsObjectNameInfoEXT info;
  info.setObjectType(objectType);
  info.setObjectHandle(handle);
  info.setPObjectName(name);

  CHK(device->setDebugUtilsObjectNameEXT(&info, DYNAMIC_DISPATCHER),
      "failed to name object");
}

template <typename T>
inline void nameObject(vk::UniqueDevice &device, T handle, const char *name) {
  _nameObject(device, handle.objectType, getVulkanHandle(handle), name);
}

template <typename T>
inline void nameObject(vk::UniqueDevice &device, T handle,
                       const std::string &name) {
  _nameObject(device, handle.objectType, getVulkanHandle(handle), name.c_str());
}
#endif // ifdef NDEBUG

#ifndef VULKING_ALLOCATOR
#define VULKING_ALLOCATOR nullptr
#endif

#ifndef ENABLE_SAMPLE_SHADING
#define ENABLE_SAMPLE_SHADING false
#endif

inline static const vk::AllocationCallbacks *ALLOCATOR = VULKING_ALLOCATOR;

static std::vector<char> readFile(const std::string &path) {
  std::ifstream file(path, std::ios::ate | std::ios::binary);

  if (!file.is_open()) {
    throw std::runtime_error(std::format("failed to open file '{}'", path));
  }

  size_t fileSize = (size_t)file.tellg();
  std::vector<char> buffer(fileSize);

  file.seekg(0);
  file.read(buffer.data(), fileSize);

  file.close();

  return buffer;
}

/* tuple(data, width, height) */
static std::tuple<std::vector<char>, uint32_t, uint32_t>
loadRgba8888Texture(const char *path) {
  int w, h, comp;
  stbi_uc *pResult = stbi_load(path, &w, &h, &comp, STBI_rgb_alpha);
  if (!pResult || comp != STBI_rgb_alpha) {
    if (pResult) {
      stbi_image_free(pResult);
    }
    throw std::runtime_error(
        std::format("failed to decompress texture: {}", path));
  }

  std::vector<char> data{};
  data.insert(end(data), pResult, pResult + w * h * 4);
  stbi_image_free(pResult);
  return {data, w, h};
}

#define MOVE_ONLY_EXPAND_ME(T)                                                 \
  T(const T &) = delete;                                                       \
  T &operator=(const T &) = delete;                                            \
  T(T &&other) noexcept {};                                                    \
  T &operator=(T &&other) noexcept {};

inline static uint32_t findMemoryType(vk::PhysicalDevice physicalDevice,
                                      uint32_t typeFilter,
                                      vk::MemoryPropertyFlags properties) {

  auto memoryProperties = physicalDevice.getMemoryProperties();
  for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
    if ((typeFilter & (1 << i)) &&
        (memoryProperties.memoryTypes[i].propertyFlags & properties) ==
            properties) {
      return i;
    }
  }

  throw std::runtime_error("failed to find suitable memory type.");
}
