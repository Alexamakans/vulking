#include <vulking/vulking.hpp>

#include <catch2/catch_all.hpp>
#include <catch2/catch_test_macros.hpp>

void dothing(GLFWwindow *window);

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

  dothing(window);

  glfwTerminate();
}

void dothing(GLFWwindow *window) {
  auto extensions = getGlfwRequiredInstanceExtensions();
  Vulking::Engine engine(window, "TestApplication", VK_MAKE_VERSION(0, 0, 1),
                         extensions);

  int src = 5;
  auto buffer = Vulking::Buffer<int>(
      &src, static_cast<vk::DeviceSize>(sizeof(src)),
      Vulking::BufferUsage::STAGING, Vulking::BufferMemory::STAGING);
  buffer.map();
  int newData = 10;
  buffer.set(&newData, sizeof(newData));
  buffer.unmap();
}
