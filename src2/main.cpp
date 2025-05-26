#include <vulking/vulking.hpp>

GLFWwindow *createWindow() {
  glfwInit();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
  GLFWwindow *window =
      CHK_GLFW(glfwCreateWindow(800, 600, "TestWindow", nullptr, nullptr),
               "failed to create GLFW window");
}

int main() { return 0; }
