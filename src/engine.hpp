#pragma once
#include <GLFW/glfw3.h>
#include <vulkan/vulkan_core.h>

#include "debug_messenger.hpp"
#include "graphics/gpu.hpp"
#include "graphics/logical_device.hpp"
#include "graphics/swapchain.hpp"

class Engine {
public:
  static GLFWwindow *window;
  static VkInstance instance;
  static VkSurfaceKHR surface;
  static VkCommandPool commandPool;

  static VkQueue graphicsQueue;
  static VkQueue presentQueue;

  static GPU gpu;
  static Swapchain swapchain;
  static LogicalDevice device;

  Engine(GLFWwindow *window);
  void init();
  ~Engine();

  static VkCommandBuffer beginCommand();
  static void endCommand(VkCommandBuffer commandBuffer, VkQueue queue);

private:
  void createInstance();

  std::vector<const char *> getRequiredExtensions();

  void createSurface();
  void pickGPU();
  void createLogicalDevice();
  void createSwapchain();

  DebugMessenger debugMessenger;
};
