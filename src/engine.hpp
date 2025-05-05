#pragma once
#include <GLFW/glfw3.h>
#include <vulkan/vulkan_core.h>

#include "debug_messenger.hpp"
#include "graphics/gpu.hpp"
#include "graphics/swapchain.hpp"

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

class Engine {
public:
  static GLFWwindow *window;
  static VkInstance instance;
  static VkSurfaceKHR surface;
  static VkDevice device;
  static VkCommandPool commandPool;

  static VkQueue graphicsQueue;
  static VkQueue presentQueue;

  static GPU gpu;
  static Swapchain swapchain;

  Engine(GLFWwindow *window);
  void init();
  ~Engine();

private:
  void createInstance();

  std::vector<const char *> getRequiredExtensions();

  void createSurface();
  void pickGPU();
  void createDevice();
  void createSwapchain();

  DebugMessenger debugMessenger;
};
