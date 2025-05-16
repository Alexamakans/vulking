#include "../Common.hpp"
#include "../Engine.hpp"
#include "../Util.hpp"
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
      CHK_GLFW(glfwCreateWindow(800, 600, "TestWindow", nullptr, nullptr),
               "failed to create GLFW window");
  REQUIRE(window != nullptr);
  CHK_GLFW(glfwIconifyWindow(window), "failed to iconify window");

  const auto extensions = getGlfwRequiredInstanceExtensions();
  Vulking::Engine engine(window, "TestApplication", VK_MAKE_VERSION(0, 0, 1),
                         extensions);

  int src = 5;
  auto buffer = engine.createBuffer(
      &src, static_cast<VkDeviceSize>(sizeof(src)),
      Vulking::Buffer::Usage::STAGING, Vulking::Buffer::Memory::STAGING);
  buffer.map();
  int newData = 10;
  buffer.set(&newData, sizeof(newData));
  buffer.unmap();
}
