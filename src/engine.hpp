#pragma once
#include <GLFW/glfw3.h>
#include <memory>
#include <new>
#include <vulkan/vulkan_core.h>

#include "debug_messenger.hpp"
#include "swapchain.hpp"

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

class Engine {
public:
  Engine(GLFWwindow *window) : window(window) {};

  void init() {
    createInstance();
    if (enableValidationLayers) {
      new (&debugMessenger) DebugMessenger;
    }
    createSurface();
  }

  ~Engine() {
    if (enableValidationLayers) {
      std::destroy_at(&debugMessenger);
    }
    vkDestroySurfaceKHR(instance, surface, allocator);
    vkDestroyInstance(instance, allocator);
    glfwDestroyWindow(window);
    glfwTerminate();
  };

private:
  void createInstance();
  void createSurface();
  void pickPhysicalDevice();
  int rateDeviceSuitability();
  void createDevice();

  GLFWwindow *window;
  VkInstance instance;
  VkSurfaceKHR surface;
  DebugMessenger debugMessenger;
  VkDevice device;
  VkPhysicalDevice physicalDevice;
};
