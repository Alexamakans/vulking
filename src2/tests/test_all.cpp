#include "../Common.hpp"
#include "../Engine.hpp"
#include "catch2/catch_test_macros.hpp"

#include <GLFW/glfw3.h>
#include <catch2/catch_all.hpp>
#include <vulkan/vulkan_core.h>

TEST_CASE("Engine engines", "[e2e]") {
  // Create window
  glfwInit();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
  GLFWwindow *window =
      glfwCreateWindow(800, 600, "TestWindow", nullptr, nullptr);
  REQUIRE(window != nullptr);
  glfwIconifyWindow(window);
  const char *description;
  int error = glfwGetError(&description);
  if (error != GLFW_NO_ERROR) {
    std::cout << std::format("glfw_error ({}): {}", error, description);
    REQUIRE(false);
  }

  // Get required extensions
  uint32_t glfwExtensionCount = 0;
  const char **glfwExtensions;
  glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

  std::vector<const char *> extensions(glfwExtensions,
                                       glfwExtensions + glfwExtensionCount);

  if (enableValidationLayers) {
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  }

  Vulking::Engine engine(window, "TestApplication", VK_MAKE_VERSION(0, 0, 1),
                         extensions);

  int src = 5;
  auto buffer = engine.createBuffer(
      &src, static_cast<VkDeviceSize>(sizeof(src)),
      Vulking::Buffer::Usage::STAGING, Vulking::Buffer::Memory::STAGING);
  REQUIRE_FALSE(buffer.isMapped());
  buffer.map();
  REQUIRE(buffer.isMapped());
  int newData = 10;
  buffer.set(&newData, sizeof(newData));
  REQUIRE(buffer.isMapped());
  buffer.unmap();
  REQUIRE_FALSE(buffer.isMapped());
}
