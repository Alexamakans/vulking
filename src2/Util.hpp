#pragma once

#include "Common.hpp"

static std::string glfwErrorCodeToString(int code) {
  switch (code) {
  case GLFW_NO_ERROR:
    return "GLFW_NO_ERROR";
  case GLFW_NOT_INITIALIZED:
    return "GLFW_NOT_INITIALIZED";
  case GLFW_NO_CURRENT_CONTEXT:
    return "GLFW_NO_CURRENT_CONTEXT";
  case GLFW_INVALID_ENUM:
    return "GLFW_INVALID_ENUM";
  case GLFW_INVALID_VALUE:
    return "GLFW_INVALID_VALUE";
  case GLFW_OUT_OF_MEMORY:
    return "GLFW_OUT_OF_MEMORY";
  case GLFW_API_UNAVAILABLE:
    return "GLFW_API_UNAVAILABLE";
  case GLFW_VERSION_UNAVAILABLE:
    return "GLFW_VERSION_UNAVAILABLE";
  case GLFW_PLATFORM_ERROR:
    return "GLFW_PLATFORM_ERROR";
  case GLFW_FORMAT_UNAVAILABLE:
    return "GLFW_FORMAT_UNAVAILABLE";
  case GLFW_NO_WINDOW_CONTEXT:
    return "GLFW_NO_WINDOW_CONTEXT";
  default:
    return std::format("UNKNOWN_GLFW_ERROR[{}]", code);
  }
}

#define CHK_GLFW(expr, msg)                                                    \
  (expr);                                                                      \
  do {                                                                         \
    const char *__description;                                                 \
    int __error = glfwGetError(&__description);                                \
    if (__error != GLFW_NO_ERROR) {                                            \
      throw std::runtime_error(                                                \
          std::format("{}:{} [{}] >{} (glfw_error: ({}) {})", __FILE__,        \
                      __LINE__, __PRETTY_FUNCTION__, msg,                      \
                      glfwErrorCodeToString(__error), __description));         \
    }                                                                          \
  } while (0)

static std::vector<const char *> getGlfwRequiredInstanceExtensions() {
  // Get required extensions
  uint32_t glfwExtensionCount = 0;
  const char **glfwExtensions;
  glfwExtensions =
      CHK_GLFW(glfwGetRequiredInstanceExtensions(&glfwExtensionCount),
               "failed to get required instance extensions");

  std::vector<const char *> extensions(glfwExtensions,
                                       glfwExtensions + glfwExtensionCount);

  if (enableValidationLayers) {
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  }

  return extensions;
}
