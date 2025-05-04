#include <chrono>
#include <cstdlib>
#include <exception>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <map>
#include <ostream>
#include <set>
#include <stdexcept>

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_funcs.hpp>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "libs/stb_image.h"

#define APP_MAJOR 0
#define APP_MINOR 0
#define APP_PATCH 1

#define ENGINE_MAJOR 0
#define ENGINE_MINOR 0
#define ENGINE_PATCH 1

#define CHK(x, msg)                                                            \
  if (x != VK_SUCCESS) {                                                       \
    throw std::runtime_error(msg);                                             \
  }

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;
const int MAX_FRAMES_IN_FLIGHT = 2;

const std::vector<const char *> validationLayers = {
    "VK_LAYER_KHRONOS_validation"};

const std::vector<const char *> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME};

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

#define STYPE(x) VK_STRUCTURE_TYPE_##x

static VKAPI_ATTR VkBool32 VKAPI_CALL
debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
              VkDebugUtilsMessageTypeFlagsEXT messageType,
              const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
              void *pUserData) {
  // if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) {
  if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
    // if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
  }
  return VK_FALSE;
}

VkResult CreateDebugUtilsMessengerEXT(
    VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
    const VkAllocationCallbacks *pAllocator,
    VkDebugUtilsMessengerEXT *pDebugMessenger) {
  auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
      instance, "vkCreateDebugUtilsMessengerEXT");
  if (func != nullptr) {
    return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
  } else {
    return VK_ERROR_EXTENSION_NOT_PRESENT;
  }
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance,
                                   VkDebugUtilsMessengerEXT debugMessenger,
                                   const VkAllocationCallbacks *pAllocator) {
  auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
      instance, "vkDestroyDebugUtilsMessengerEXT");
  if (func != nullptr) {
    func(instance, debugMessenger, pAllocator);
  }
}

struct QueueFamilyIndices {
  std::optional<uint32_t> graphicsFamily;
  std::optional<uint32_t> presentFamily;

  bool isComplete() {
    return graphicsFamily.has_value() && presentFamily.has_value();
  }
};

struct SwapChainSupportDetails {
  VkSurfaceCapabilitiesKHR capabilities;
  std::vector<VkSurfaceFormatKHR> formats;
  std::vector<VkPresentModeKHR> presentModes;
};

struct Vertex {
  glm::vec3 pos;
  glm::vec3 color;
  glm::vec2 texCoord;

  static VkVertexInputBindingDescription getBindingDescription() {
    VkVertexInputBindingDescription bindingDescription{};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(Vertex);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    return bindingDescription;
  }

  static std::array<VkVertexInputAttributeDescription, 3>
  getAttributeDescriptions() {
    std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};
    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(Vertex, pos);

    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(Vertex, color);

    attributeDescriptions[2].binding = 0;
    attributeDescriptions[2].location = 2;
    attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

    return attributeDescriptions;
  }
};

struct UniformBufferObject {
  alignas(4) glm::float32 time;
  alignas(16) glm::mat4 model;
  alignas(16) glm::mat4 view;
  alignas(16) glm::mat4 proj;
};

const std::vector<Vertex> vertices = {
    {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
    {{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
    {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
    {{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}},
};
const std::vector<uint32_t> indices = {
    0, 1, 2, 2, 3, 0,
};

class MyApplication {
public:
  void run() {
    initWindow();
    initVulkan();
    mainLoop();
    cleanup();
  }

private:
  GLFWwindow *window;
  VkInstance instance;
  VkDebugUtilsMessengerEXT debugMessenger;

  VkSurfaceKHR surface;

  VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
  VkDevice device;

  VkQueue graphicsQueue;
  VkQueue presentQueue;

  VkSwapchainKHR swapChain;
  std::vector<VkImage> swapChainImages;
  std::vector<VkImageView> swapChainImageViews;
  std::vector<VkFramebuffer> swapChainFramebuffers;
  VkFormat swapChainImageFormat;
  VkExtent2D swapChainExtent;

  VkRenderPass renderPass;
  VkDescriptorSetLayout descriptorSetLayout;
  VkPipelineLayout pipelineLayout;
  VkPipeline graphicsPipeline;

  VkDescriptorPool descriptorPool;
  std::vector<VkDescriptorSet> descriptorSets;

  VkCommandPool commandPool;
  std::vector<VkCommandBuffer> commandBuffers;

  VkSampler textureSampler;

  VkImage textureImage;
  VkDeviceMemory textureImageMemory;

  VkImageView textureImageView;

  VkBuffer vertexBuffer;
  VkDeviceMemory vertexBufferMemory;

  VkBuffer indexBuffer;
  VkDeviceMemory indexBufferMemory;

  std::vector<VkBuffer> uniformBuffers;
  std::vector<VkDeviceMemory> uniformBuffersMemory;
  std::vector<void *> uniformBuffersMapped;

  std::vector<VkSemaphore> imageAvailableSemaphores;
  std::vector<VkSemaphore> renderFinishedSemaphores;
  std::vector<VkFence> inFlightFences;

  bool framebufferResized = false;

  uint32_t frameIndex = 0;

  void initWindow() {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    window = glfwCreateWindow(WIDTH, HEIGHT, "Vulking", nullptr, nullptr);
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, onFramebufferResize);
  }

  static void onFramebufferResize(GLFWwindow *window, int width, int height) {
    auto app =
        reinterpret_cast<MyApplication *>(glfwGetWindowUserPointer(window));
    app->framebufferResized = true;
  }

  void initVulkan() {
    printExtensions();
    createInstance();
    setupDebugMessenger();
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();

    createSwapChain();
    createImageViews();
    createRenderPass();
    createDescriptorSetLayout();
    createGraphicsPipeline();
    createFramebuffers();

    createCommandPool();

    createTextureImage();
    createTextureImageView();
    createTextureSampler();

    createVertexBuffer();
    createIndexBuffer();
    createUniformBuffers();

    createDescriptorPool();
    createDescriptorSets();

    createCommandBuffer();
    createSyncObjects();
  }

  void printExtensions() {
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount,
                                           extensions.data());

    std::cout << "available extensions:\n";
    for (const auto &extension : extensions) {
      std::cout << '\t' << extension.extensionName << std::endl;
    }
  }

  void createInstance() {
    if (enableValidationLayers && !checkValidationLayerSupport()) {
      throw std::runtime_error(
          "validation layers requested, but not available");
    }

    VkApplicationInfo appInfo{};
    appInfo.sType = STYPE(APPLICATION_INFO);
    appInfo.pApplicationName = "Fakking Vulking";
    appInfo.applicationVersion =
        VK_MAKE_VERSION(APP_MAJOR, APP_MINOR, APP_PATCH);
    appInfo.pEngineName = "VROOM";
    appInfo.engineVersion =
        VK_MAKE_VERSION(ENGINE_MAJOR, ENGINE_MINOR, ENGINE_PATCH);
    appInfo.apiVersion = VK_API_VERSION_1_3;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = STYPE(INSTANCE_CREATE_INFO);
    createInfo.pApplicationInfo = &appInfo;

    auto extensions = getRequiredExtensions();

    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    std::cout << "required extensions:\n";
    for (const auto &extension : extensions) {
      std::cout << '\t' << extension << std::endl;
    }

    if (enableValidationLayers) {
      createInfo.enabledLayerCount =
          static_cast<uint32_t>(validationLayers.size());
      createInfo.ppEnabledLayerNames = validationLayers.data();
    } else {
      createInfo.enabledLayerCount = 0;
    }

    CHK(vkCreateInstance(&createInfo, nullptr, &instance),
        "failed to create instance")
  }

  bool checkValidationLayerSupport() {
    uint32_t layerCount;
    CHK(vkEnumerateInstanceLayerProperties(&layerCount, nullptr),
        "failed getting layer count")

    std::vector<VkLayerProperties> availableLayers(layerCount);
    CHK(vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data()),
        "failed getting layer properties")

    for (const char *layerName : validationLayers) {
      bool layerFound = false;

      for (const auto &layerProperties : availableLayers) {
        if (strcmp(layerName, layerProperties.layerName) == 0) {
          layerFound = true;
          break;
        }
      }

      if (!layerFound) {
        std::cerr << "layer " << std::quoted(layerName) << " not found"
                  << std::endl;
        return false;
      }
    }

    return true;
  }

  std::vector<const char *> getRequiredExtensions() {
    uint32_t glfwExtensionCount = 0;
    const char **glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char *> extensions(glfwExtensions,
                                         glfwExtensions + glfwExtensionCount);

    if (enableValidationLayers) {
      extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
  }

  void setupDebugMessenger() {
    if (!enableValidationLayers)
      return;

    VkDebugUtilsMessengerCreateInfoEXT createInfo{};
    createInfo.sType = STYPE(DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT);
    createInfo.messageSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
    createInfo.pUserData = nullptr; // Optional

    CHK(CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr,
                                     &debugMessenger),
        "failed to set up debug messenger")
  }

  void createSurface() {
    CHK(glfwCreateWindowSurface(instance, window, nullptr, &surface),
        "failed to create window surface!")
  }

  void pickPhysicalDevice() {
    uint32_t deviceCount = 0;
    CHK(vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr),
        "failed to get physical device count")

    if (deviceCount == 0) {
      throw std::runtime_error("failed to find GPUs with Vulkan support");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    CHK(vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data()),
        "failed to enumerate physical devices")

    std::multimap<int, VkPhysicalDevice> candidates;

    for (const auto &device : devices) {
      int score = rateDeviceSuitability(device);
      candidates.insert(std::make_pair(score, device));
    }

    if (candidates.rbegin()->first > 0) {
      physicalDevice = candidates.rbegin()->second;
    } else {
      throw std::runtime_error("failed to find a suitable GPU");
    }
  }

  int rateDeviceSuitability(VkPhysicalDevice device) {
    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

    int score = 0;

    if (!deviceFeatures.geometryShader) {
      // Application can't function without geometry shaders
      return 0;
    }

    if (!deviceFeatures.samplerAnisotropy) {
      return 0;
    }

    QueueFamilyIndices indices = findQueueFamilies(device);
    if (!indices.isComplete()) {
      return 0;
    }

    if (!checkDeviceExtensionSupport(device)) {
      return 0;
    }

    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
    bool swapChainAdequate = !swapChainSupport.formats.empty() &&
                             !swapChainSupport.presentModes.empty();

    // Discrete GPUs have a significant performance advantage
    if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
      score += 1000;
    }

    // Maximum possible size of textures affects graphics quality
    score += deviceProperties.limits.maxImageDimension2D;

    return score;
  }

  QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) {
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount,
                                             nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount,
                                             queueFamilies.data());

    int i = 0;
    for (const auto &queueFamily : queueFamilies) {
      if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
        indices.graphicsFamily = i;
      }

      VkBool32 presentSupport = false;
      vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
      if (presentSupport) {
        indices.presentFamily = i;
      }

      if (indices.isComplete()) {
        break;
      }
      i++;
    }

    return indices;
  }

  bool checkDeviceExtensionSupport(VkPhysicalDevice device) {
    uint32_t extensionCount;
    CHK(vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount,
                                             nullptr),
        "failed to get device extension properties count")

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    CHK(vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount,
                                             availableExtensions.data()),
        "failed to enumerate device extension properties")

    std::set<std::string> requiredExtensions(deviceExtensions.begin(),
                                             deviceExtensions.end());

    for (const auto &extension : availableExtensions) {
      requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
  }

  void createLogicalDevice() {
    QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(),
                                              indices.presentFamily.value()};

    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies) {
      VkDeviceQueueCreateInfo queueCreateInfo{};
      queueCreateInfo.sType = STYPE(DEVICE_QUEUE_CREATE_INFO);
      queueCreateInfo.queueFamilyIndex = indices.graphicsFamily.value();
      queueCreateInfo.queueCount = 1;
      queueCreateInfo.pQueuePriorities = &queuePriority;

      queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures{
        .samplerAnisotropy = VK_TRUE,
    };

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = STYPE(DEVICE_CREATE_INFO);
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.queueCreateInfoCount =
        static_cast<uint32_t>(queueCreateInfos.size());

    createInfo.pEnabledFeatures = &deviceFeatures;

    createInfo.enabledExtensionCount =
        static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    if (enableValidationLayers) {
      createInfo.enabledLayerCount =
          static_cast<uint32_t>(validationLayers.size());
      createInfo.ppEnabledLayerNames = validationLayers.data();
    } else {
      createInfo.enabledLayerCount = 0;
    }

    CHK(vkCreateDevice(physicalDevice, &createInfo, nullptr, &device),
        "failed to create logical device")

    vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
    vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
  }

  void createSwapChain() {
    SwapChainSupportDetails swapChainSupport =
        querySwapChainSupport(physicalDevice);
    VkSurfaceFormatKHR surfaceFormat =
        chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode =
        chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 &&
        imageCount > swapChainSupport.capabilities.maxImageCount) {
      imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = STYPE(SWAPCHAIN_CREATE_INFO_KHR);
    createInfo.surface = surface;

    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
    uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(),
                                     indices.presentFamily.value()};

    if (indices.graphicsFamily != indices.presentFamily) {
      createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
      createInfo.queueFamilyIndexCount = 2;
      createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
      createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
      createInfo.queueFamilyIndexCount = 0;
      createInfo.pQueueFamilyIndices = nullptr;
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;

    createInfo.oldSwapchain = VK_NULL_HANDLE;

    CHK(vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain),
        "failed to create swap chain")

    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
    swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(device, swapChain, &imageCount,
                            swapChainImages.data());

    swapChainImageFormat = surfaceFormat.format;
    swapChainExtent = extent;
  }

  VkSurfaceFormatKHR chooseSwapSurfaceFormat(
      const std::vector<VkSurfaceFormatKHR> &availableFormats) {
    for (const auto &availableFormat : availableFormats) {
      if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
          availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
        return availableFormat;
      }
    }

    return availableFormats[0];
  }

  VkPresentModeKHR chooseSwapPresentMode(
      const std::vector<VkPresentModeKHR> &availablePresentModes) {
    for (const auto &availablePresentMode : availablePresentModes) {
      if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
        return availablePresentMode;
      }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
  }

  VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities) {
    if (capabilities.currentExtent.width !=
        std::numeric_limits<uint32_t>::max()) {
      return capabilities.currentExtent;
    } else {
      int width, height;
      glfwGetFramebufferSize(window, &width, &height);

      VkExtent2D actualExtent = {
          static_cast<uint32_t>(width),
          static_cast<uint32_t>(height),
      };

      actualExtent.width =
          std::clamp(actualExtent.width, capabilities.minImageExtent.width,
                     capabilities.maxImageExtent.width);
      actualExtent.height =
          std::clamp(actualExtent.height, capabilities.minImageExtent.height,
                     capabilities.maxImageExtent.height);
      return actualExtent;
    }
  }

  SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device) {
    SwapChainSupportDetails swapChainSupport;

    CHK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
            device, surface, &swapChainSupport.capabilities),
        "failed to get physical device surface capabilities")

    uint32_t formatCount;
    CHK(vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount,
                                             nullptr),
        "failed to get physical device surface formats count");

    if (formatCount != 0) {
      swapChainSupport.formats.resize(formatCount);
      CHK(vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount,
                                               swapChainSupport.formats.data()),
          "failed to get physical device surface formats");
    }

    uint32_t presentModeCount;
    CHK(vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface,
                                                  &presentModeCount, nullptr),
        "failed to get physical device surface present modes count");
    if (presentModeCount != 0) {
      swapChainSupport.presentModes.resize(presentModeCount);
      CHK(vkGetPhysicalDeviceSurfacePresentModesKHR(
              device, surface, &presentModeCount,
              swapChainSupport.presentModes.data()),
          "failed to get physical device surface present modes");
    }

    return swapChainSupport;
  }

  void createImageViews() {
    swapChainImageViews.resize(swapChainImages.size());
    for (size_t i = 0; i < swapChainImages.size(); i++) {
      swapChainImageViews[i] =
          createImageView(swapChainImages[i], swapChainImageFormat);
    }
  }

  void createRenderPass() {
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = swapChainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;

    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    VkRenderPassCreateInfo renderPassCreateInfo{};
    renderPassCreateInfo.sType = STYPE(RENDER_PASS_CREATE_INFO);
    renderPassCreateInfo.attachmentCount = 1;
    renderPassCreateInfo.pAttachments = &colorAttachment;
    renderPassCreateInfo.subpassCount = 1;
    renderPassCreateInfo.pSubpasses = &subpass;

    CHK(vkCreateRenderPass(device, &renderPassCreateInfo, nullptr, &renderPass),
        "failed to create render pass")
  }

  void createDescriptorSetLayout() {
    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    uboLayoutBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding samplerLayoutBinding{
        .binding = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
        .pImmutableSamplers = nullptr,
    };

    std::array<VkDescriptorSetLayoutBinding, 2> bindings = {
        uboLayoutBinding, samplerLayoutBinding};

    VkDescriptorSetLayoutCreateInfo layoutCreateInfo{};
    layoutCreateInfo.sType = STYPE(DESCRIPTOR_SET_LAYOUT_CREATE_INFO);
    layoutCreateInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutCreateInfo.pBindings = bindings.data();

    CHK(vkCreateDescriptorSetLayout(device, &layoutCreateInfo, nullptr,
                                    &descriptorSetLayout),
        "failed to create descriptor set layout")
  }

  void createGraphicsPipeline() {
    auto vertShaderCode = readFile("assets/shaders/test.vert.spv");
    auto fragShaderCode = readFile("assets/shaders/test.frag.spv");

    auto vertShaderModule = createShaderModule(vertShaderCode);
    auto fragShaderModule = createShaderModule(fragShaderCode);

    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = STYPE(PIPELINE_SHADER_STAGE_CREATE_INFO);
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";
    // pSpecializationInfo can be used to set compile-time constants, allowing
    // for optimizations like eliminating branches that won't be executed etc.
    vertShaderStageInfo.pSpecializationInfo = nullptr;

    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType = STYPE(PIPELINE_SHADER_STAGE_CREATE_INFO);
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";
    // pSpecializationInfo can be used to set compile-time constants, allowing
    // for optimizations like eliminating branches that won't be executed etc.
    fragShaderStageInfo.pSpecializationInfo = nullptr;

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo,
                                                      fragShaderStageInfo};

    auto bindingDescription = Vertex::getBindingDescription();
    auto attributeDescriptions = Vertex::getAttributeDescriptions();

    VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo{};
    vertexInputCreateInfo.sType =
        STYPE(PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO);
    vertexInputCreateInfo.vertexBindingDescriptionCount = 1;
    vertexInputCreateInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputCreateInfo.vertexAttributeDescriptionCount =
        static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputCreateInfo.pVertexAttributeDescriptions =
        attributeDescriptions.data();

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo{};
    inputAssemblyCreateInfo.sType =
        STYPE(PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO);
    inputAssemblyCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssemblyCreateInfo.primitiveRestartEnable = VK_FALSE;

    VkPipelineViewportStateCreateInfo viewportStateCreateInfo{};
    viewportStateCreateInfo.sType = STYPE(PIPELINE_VIEWPORT_STATE_CREATE_INFO);
    viewportStateCreateInfo.viewportCount = 1;
    viewportStateCreateInfo.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo rasterizerCreateInfo{};
    rasterizerCreateInfo.sType =
        STYPE(PIPELINE_RASTERIZATION_STATE_CREATE_INFO);
    rasterizerCreateInfo.depthClampEnable = VK_FALSE;
    rasterizerCreateInfo.rasterizerDiscardEnable = VK_FALSE;
    rasterizerCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizerCreateInfo.lineWidth = 1.0f;
    rasterizerCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizerCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizerCreateInfo.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisamplingCreateInfo{};
    multisamplingCreateInfo.sType =
        STYPE(PIPELINE_MULTISAMPLE_STATE_CREATE_INFO);
    multisamplingCreateInfo.sampleShadingEnable = VK_FALSE;
    multisamplingCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendAttachmentState colorBlendAttachmentCreateInfo{};
    colorBlendAttachmentCreateInfo.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachmentCreateInfo.blendEnable = VK_TRUE;
    colorBlendAttachmentCreateInfo.srcColorBlendFactor =
        VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachmentCreateInfo.dstColorBlendFactor =
        VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachmentCreateInfo.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachmentCreateInfo.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachmentCreateInfo.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachmentCreateInfo.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo colorBlendCreateInfo{};
    colorBlendCreateInfo.sType = STYPE(PIPELINE_COLOR_BLEND_STATE_CREATE_INFO);
    colorBlendCreateInfo.logicOpEnable = VK_FALSE;
    colorBlendCreateInfo.logicOp = VK_LOGIC_OP_COPY;
    colorBlendCreateInfo.attachmentCount = 1;
    colorBlendCreateInfo.pAttachments = &colorBlendAttachmentCreateInfo;
    colorBlendCreateInfo.blendConstants[0] = 0.0f;
    colorBlendCreateInfo.blendConstants[1] = 0.0f;
    colorBlendCreateInfo.blendConstants[2] = 0.0f;
    colorBlendCreateInfo.blendConstants[3] = 0.0f;

    std::vector<VkDynamicState> dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR,
    };
    VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo{};
    dynamicStateCreateInfo.sType = STYPE(PIPELINE_DYNAMIC_STATE_CREATE_INFO);
    dynamicStateCreateInfo.dynamicStateCount =
        static_cast<uint32_t>(dynamicStates.size());
    dynamicStateCreateInfo.pDynamicStates = dynamicStates.data();

    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
    pipelineLayoutCreateInfo.sType = STYPE(PIPELINE_LAYOUT_CREATE_INFO);
    // Define uniforms here
    pipelineLayoutCreateInfo.setLayoutCount = 1;
    pipelineLayoutCreateInfo.pSetLayouts = &descriptorSetLayout;
    pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
    pipelineLayoutCreateInfo.pPushConstantRanges = nullptr;

    CHK(vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr,
                               &pipelineLayout),
        "failed to create pipeline layout")

    VkGraphicsPipelineCreateInfo pipelineCreateInfo{};
    pipelineCreateInfo.sType = STYPE(GRAPHICS_PIPELINE_CREATE_INFO);
    pipelineCreateInfo.stageCount = 2;
    pipelineCreateInfo.pStages = shaderStages;

    pipelineCreateInfo.pVertexInputState = &vertexInputCreateInfo;
    pipelineCreateInfo.pInputAssemblyState = &inputAssemblyCreateInfo;
    pipelineCreateInfo.pViewportState = &viewportStateCreateInfo;
    pipelineCreateInfo.pRasterizationState = &rasterizerCreateInfo;
    pipelineCreateInfo.pMultisampleState = &multisamplingCreateInfo;
    pipelineCreateInfo.pDepthStencilState = nullptr;
    pipelineCreateInfo.pColorBlendState = &colorBlendCreateInfo;
    pipelineCreateInfo.pDynamicState = &dynamicStateCreateInfo;

    pipelineCreateInfo.layout = pipelineLayout;
    pipelineCreateInfo.renderPass = renderPass;
    pipelineCreateInfo.subpass = 0; // index

    // The following can be used to derive from an existing pipeline.
    // For these to be used, the VK_PIPELINE_CREATE_DERIVATIVE_BIT must be set
    // on the VkGraphicsPipelineCreateInfo.flags property.
    pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineCreateInfo.basePipelineIndex = -1;

    CHK(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1,
                                  &pipelineCreateInfo, nullptr,
                                  &graphicsPipeline),
        "failed to create graphics pipeline")

    vkDestroyShaderModule(device, vertShaderModule, nullptr);
    vkDestroyShaderModule(device, fragShaderModule, nullptr);
  }

  VkShaderModule createShaderModule(const std::vector<char> &code) {
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = STYPE(SHADER_MODULE_CREATE_INFO);
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());

    VkShaderModule shaderModule;
    CHK(vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule),
        "failed to create shader module")

    return shaderModule;
  }

  void createFramebuffers() {
    swapChainFramebuffers.resize(swapChainImageViews.size());

    for (size_t i = 0; i < swapChainImageViews.size(); i++) {
      VkImageView attachments[] = {swapChainImageViews[i]};

      VkFramebufferCreateInfo createInfo{};
      createInfo.sType = STYPE(FRAMEBUFFER_CREATE_INFO);
      createInfo.renderPass = renderPass;
      createInfo.attachmentCount = 1;
      createInfo.pAttachments = attachments;
      createInfo.width = swapChainExtent.width;
      createInfo.height = swapChainExtent.height;
      createInfo.layers = 1;

      CHK(vkCreateFramebuffer(device, &createInfo, nullptr,
                              &swapChainFramebuffers[i]),
          "failed to create framebuffer")
    }
  }

  void createCommandPool() {
    auto queueFamilyIndices = findQueueFamilies(physicalDevice);
    VkCommandPoolCreateInfo createInfo{};
    createInfo.sType = STYPE(COMMAND_POOL_CREATE_INFO);
    createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    createInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    CHK(vkCreateCommandPool(device, &createInfo, nullptr, &commandPool),
        "failed to create command pool")
  }

  void createTextureImage() {
    int width, height, channels;
    stbi_uc *pixels = stbi_load("assets/textures/texture.jpg", &width, &height,
                                &channels, STBI_rgb_alpha);
    if (!pixels) {
      throw std::runtime_error("failed to load texture image");
    }

    VkDeviceSize imageSize = width * height * 4;
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createAndInitStagingBuffer(pixels, imageSize, stagingBuffer,
                               stagingBufferMemory);
    stbi_image_free(pixels);

    createImage(width, height, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage,
                textureImageMemory);

    transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB,
                          VK_IMAGE_LAYOUT_UNDEFINED,
                          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    copyBufferToImage(stagingBuffer, textureImage, static_cast<uint32_t>(width),
                      static_cast<uint32_t>(height));
    transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB,
                          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                          VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
  }

  void createTextureImageView() {
    textureImageView = createImageView(textureImage, VK_FORMAT_R8G8B8A8_SRGB);
  }

  VkImageView createImageView(VkImage image, VkFormat format) {
    VkImageViewCreateInfo createInfo{};
    createInfo.sType = STYPE(IMAGE_VIEW_CREATE_INFO);

    createInfo.image = image;
    createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    createInfo.format = format;

    createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

    createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    createInfo.subresourceRange.baseMipLevel = 0;
    createInfo.subresourceRange.levelCount = 1;
    createInfo.subresourceRange.baseArrayLayer = 0;
    createInfo.subresourceRange.layerCount = 1;

    VkImageView imageView;
    CHK(vkCreateImageView(device, &createInfo, nullptr, &imageView),
        "failed to create texture image view");
    return imageView;
  }

  void createImage(uint32_t width, uint32_t height, VkFormat format,
                   VkImageTiling tiling, VkImageUsageFlags usage,
                   VkMemoryPropertyFlags properties, VkImage &image,
                   VkDeviceMemory &imageMemory) {
    VkImageCreateInfo imageCreateInfo{
        .sType = STYPE(IMAGE_CREATE_INFO),
        .imageType = VK_IMAGE_TYPE_2D,
        .format = format,
        .extent{
            .width = width,
            .height = height,
            .depth = 1,
        },
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = tiling,
        .usage = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    };

    CHK(vkCreateImage(device, &imageCreateInfo, nullptr, &image),
        "failed to create image")

    VkMemoryRequirements memoryRequirements;
    vkGetImageMemoryRequirements(device, image, &memoryRequirements);

    VkMemoryAllocateInfo allocInfo{
        .sType = STYPE(MEMORY_ALLOCATE_INFO),
        .allocationSize = memoryRequirements.size,
        .memoryTypeIndex =
            findMemoryType(memoryRequirements.memoryTypeBits, properties),
    };

    CHK(vkAllocateMemory(device, &allocInfo, nullptr, &imageMemory),
        "failed to allocate image memory")

    vkBindImageMemory(device, image, imageMemory, 0);
  }

  void transitionImageLayout(VkImage image, VkFormat format,
                             VkImageLayout oldLayout, VkImageLayout newLayout) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    VkImageMemoryBarrier barrier{
        .sType = STYPE(IMAGE_MEMORY_BARRIER),
        .oldLayout = oldLayout,
        .newLayout = newLayout,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = image,
        .subresourceRange =
            {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
    };

    VkPipelineStageFlags srcStage;
    VkPipelineStageFlags dstStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
        newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
      barrier.srcAccessMask = 0;
      barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

      srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
      dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
               newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
      barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
      barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

      srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
      dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else {
      throw std::runtime_error("unsupported layer transition");
    }

    vkCmdPipelineBarrier(commandBuffer, srcStage, dstStage, 0, 0, nullptr, 0,
                         nullptr, 1, &barrier);

    endSingleTimeCommands(commandBuffer);
  }

  void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width,
                         uint32_t height) {
    auto commandBuffer = beginSingleTimeCommands();

    VkBufferImageCopy region{
        .bufferOffset = 0,
        .bufferRowLength = 0,
        .bufferImageHeight = 0,
        .imageSubresource =
            {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .mipLevel = 0,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
        .imageOffset = {0, 0, 0},
        .imageExtent = {width, height, 1},
    };

    vkCmdCopyBufferToImage(commandBuffer, buffer, image,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    endSingleTimeCommands(commandBuffer);
  }

  void createTextureSampler() {
    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(physicalDevice, &properties);

    VkSamplerCreateInfo samplerCreateInfo{
        .sType = STYPE(SAMPLER_CREATE_INFO),
        .magFilter = VK_FILTER_LINEAR,
        .minFilter = VK_FILTER_LINEAR,
        .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
        .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .mipLodBias = 0.0f,
        .anisotropyEnable = VK_TRUE,
        .maxAnisotropy = properties.limits.maxSamplerAnisotropy,
        .compareEnable = VK_FALSE,
        .compareOp = VK_COMPARE_OP_ALWAYS,
        .minLod = 0.0f,
        .maxLod = 0.0f,
        .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
        .unnormalizedCoordinates = VK_FALSE,
    };

    CHK(vkCreateSampler(device, &samplerCreateInfo, nullptr, &textureSampler),
        "failed to create texture sampler")
  }

  void createVertexBuffer() {
    VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                     VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 stagingBuffer, stagingBufferMemory);

    void *data;
    CHK(vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data),
        "failed to map staging buffer memory")
    memcpy(data, vertices.data(), (size_t)bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);

    createBuffer(
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);

    copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
  }

  void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
                    VkMemoryPropertyFlags properties, VkBuffer &buffer,
                    VkDeviceMemory &bufferMemory) {
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = STYPE(BUFFER_CREATE_INFO);
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    CHK(vkCreateBuffer(device, &bufferInfo, nullptr, &buffer),
        "failed to create vertex buffer");

    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements(device, buffer, &memoryRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = STYPE(MEMORY_ALLOCATE_INFO);
    allocInfo.allocationSize = memoryRequirements.size;
    allocInfo.memoryTypeIndex =
        findMemoryType(memoryRequirements.memoryTypeBits,
                       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                           VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    CHK(vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory),
        "failed to allocate vertex buffer memory")

    CHK(vkBindBufferMemory(device, buffer, bufferMemory, 0),
        "failed to bind vertex buffer memory")
  }

  // returns a void * that is mapped to outMemory
  void createAndInitStagingBuffer(void *data, VkDeviceSize bufferSize,
                                  VkBuffer &stagingBuffer,
                                  VkDeviceMemory &stagingBufferMemory) {
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                     VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 stagingBuffer, stagingBufferMemory);
    void *dst;
    CHK(vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &dst),
        "failed to map staging buffer memory")
    memcpy(dst, data, (size_t)bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);
  }

  void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
    auto commandBuffer = beginSingleTimeCommands();

    VkBufferCopy copyRegion{};
    copyRegion.srcOffset = 0;
    copyRegion.dstOffset = 0;
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    endSingleTimeCommands(commandBuffer);
  }

  uint32_t findMemoryType(uint32_t typeFilter,
                          VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memoryProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);

    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
      if ((typeFilter & (1 << i)) &&
          (memoryProperties.memoryTypes[i].propertyFlags & properties) ==
              properties) {
        return i;
      }
    }

    throw std::runtime_error("failed to find suitable memory type");
  }

  void createIndexBuffer() {
    VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                     VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 stagingBuffer, stagingBufferMemory);

    void *data;
    CHK(vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data),
        "failed to map staging buffer memory")
    memcpy(data, indices.data(), (size_t)bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);

    createBuffer(
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory);

    copyBuffer(stagingBuffer, indexBuffer, bufferSize);

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
  }

  void createUniformBuffers() {
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);

    uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
    uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
      createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                       VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                   uniformBuffers[i], uniformBuffersMemory[i]);

      CHK(vkMapMemory(device, uniformBuffersMemory[i], 0, bufferSize, 0,
                      &uniformBuffersMapped[i]),
          std::format("failed to map uniform buffer memory {}", i))
    }
  }

  void createDescriptorPool() {
    std::array<VkDescriptorPoolSize, 2> poolSizes{};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    VkDescriptorPoolCreateInfo poolCreateInfo{};
    poolCreateInfo.sType = STYPE(DESCRIPTOR_POOL_CREATE_INFO);
    poolCreateInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolCreateInfo.pPoolSizes = poolSizes.data();
    poolCreateInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    CHK(vkCreateDescriptorPool(device, &poolCreateInfo, nullptr,
                               &descriptorPool),
        "failed to create descriptor pool")
  }

  void createDescriptorSets() {
    std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT,
                                               descriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = STYPE(DESCRIPTOR_SET_ALLOCATE_INFO);
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    allocInfo.pSetLayouts = layouts.data();

    descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
    CHK(vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()),
        "failed to allocate descriptor sets")

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
      VkDescriptorBufferInfo bufferInfo{
          .buffer = uniformBuffers[i],
          .offset = 0,
          .range = sizeof(UniformBufferObject),
      };

      VkDescriptorImageInfo imageInfo{
          .sampler = textureSampler,
          .imageView = textureImageView,
          .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
      };

      std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

      descriptorWrites[0] = {
          .sType = STYPE(WRITE_DESCRIPTOR_SET),
          .dstSet = descriptorSets[i],
          .dstBinding = 0,
          .dstArrayElement = 0,
          .descriptorCount = 1,
          .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
          .pBufferInfo = &bufferInfo,
      };

      descriptorWrites[1] = {
          .sType = STYPE(WRITE_DESCRIPTOR_SET),
          .dstSet = descriptorSets[i],
          .dstBinding = 1,
          .dstArrayElement = 0,
          .descriptorCount = 1,
          .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
          .pImageInfo = &imageInfo,
      };

      vkUpdateDescriptorSets(device,
                             static_cast<uint32_t>(descriptorWrites.size()),
                             descriptorWrites.data(), 0, nullptr);
    }
  }

  void createCommandBuffer() {
    commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = STYPE(COMMAND_BUFFER_ALLOCATE_INFO);
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

    CHK(vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()),
        "failed to allocate command buffers")
  }

  void createSyncObjects() {
    imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphoreCreateInfo{};
    semaphoreCreateInfo.sType = STYPE(SEMAPHORE_CREATE_INFO);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
      CHK(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr,
                            &imageAvailableSemaphores[i]),
          std::format("failed to create image available semaphore {}", i))
      CHK(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr,
                            &renderFinishedSemaphores[i]),
          std::format("failed to create render finished semaphore {}", i))

      VkFenceCreateInfo fenceCreateInfo{};
      fenceCreateInfo.sType = STYPE(FENCE_CREATE_INFO);
      fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
      CHK(vkCreateFence(device, &fenceCreateInfo, nullptr, &inFlightFences[i]),
          std::format("failed to create in flight fence {}", i))
    }
  }

  void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
    const size_t frame = frameIndex % MAX_FRAMES_IN_FLIGHT;

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = STYPE(COMMAND_BUFFER_BEGIN_INFO);
    beginInfo.flags = 0;
    beginInfo.pInheritanceInfo = nullptr;

    CHK(vkBeginCommandBuffer(commandBuffer, &beginInfo),
        "failed to begin recording command buffer")

    VkRenderPassBeginInfo renderPassBeginInfo{};
    renderPassBeginInfo.sType = STYPE(RENDER_PASS_BEGIN_INFO);
    renderPassBeginInfo.renderPass = renderPass;
    renderPassBeginInfo.framebuffer = swapChainFramebuffers[imageIndex];
    renderPassBeginInfo.renderArea.offset = {0, 0};
    renderPassBeginInfo.renderArea.extent = swapChainExtent;

    VkClearValue clearColor = {{{0.0f, 0.05f, 0.25f, 1.0f}}};
    renderPassBeginInfo.clearValueCount = 1;
    renderPassBeginInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo,
                         VK_SUBPASS_CONTENTS_INLINE);
    {
      vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                        graphicsPipeline);

      VkViewport viewport{};
      viewport.x = 0.0f;
      viewport.y = 0.0f;
      viewport.width = (float)swapChainExtent.width;
      viewport.height = (float)swapChainExtent.height;
      viewport.minDepth = 0.0f;
      viewport.maxDepth = 1.0f;
      vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

      VkRect2D scissor{};
      scissor.offset = {0, 0};
      scissor.extent = swapChainExtent;
      vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

      VkBuffer vertexBuffers[] = {vertexBuffer};
      VkDeviceSize offsets[] = {0};
      vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
      vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);

      vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                              pipelineLayout, 0, 1, &descriptorSets[frame], 0,
                              nullptr);

      vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indices.size()), 1,
                       0, 0, 0);
    }
    vkCmdEndRenderPass(commandBuffer);

    CHK(vkEndCommandBuffer(commandBuffer), "failed to record command buffer")
  }

  VkCommandBuffer beginSingleTimeCommands() {
    VkCommandBufferAllocateInfo allocInfo{
        .sType = STYPE(COMMAND_BUFFER_ALLOCATE_INFO),
        .commandPool = commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };

    VkCommandBuffer commandBuffer;
    CHK(vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer),
        "failed to allocate command buffer")

    VkCommandBufferBeginInfo beginInfo{
        .sType = STYPE(COMMAND_BUFFER_BEGIN_INFO),
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };

    CHK(vkBeginCommandBuffer(commandBuffer, &beginInfo),
        "failed to begin command buffer")

    return commandBuffer;
  }

  void endSingleTimeCommands(VkCommandBuffer commandBuffer) {
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{
        .sType = STYPE(SUBMIT_INFO),
        .commandBufferCount = 1,
        .pCommandBuffers = &commandBuffer,
    };

    CHK(vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE),
        "failed to submit command buffer to queue")
    CHK(vkQueueWaitIdle(graphicsQueue), "failed to wait for queue to be idle")

    vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
  }

  void mainLoop() {
    while (!glfwWindowShouldClose(window)) {
      glfwPollEvents();
      drawFrame();
    }

    vkDeviceWaitIdle(device);
  }

  void drawFrame() {
    const uint32_t frame = frameIndex % MAX_FRAMES_IN_FLIGHT;
    CHK(vkWaitForFences(device, 1, &inFlightFences[frame], VK_TRUE, UINT64_MAX),
        "failed waiting for in flight fence")

    uint32_t imageIndex;
    VkResult acquireResult = vkAcquireNextImageKHR(
        device, swapChain, UINT64_MAX, imageAvailableSemaphores[frame],
        VK_NULL_HANDLE, &imageIndex);
    if (acquireResult == VK_ERROR_OUT_OF_DATE_KHR) {
      recreateSwapChain();
      return;
    } else if (acquireResult != VK_SUCCESS &&
               acquireResult != VK_SUBOPTIMAL_KHR) {
      throw std::runtime_error("failed to acquire swap chain image");
    }

    CHK(vkResetFences(device, 1, &inFlightFences[frame]),
        "failed resetting in flight fence")

    updateUniformBuffer(frame);

    VkCommandBufferResetFlags flags = 0;
    CHK(vkResetCommandBuffer(commandBuffers[frame], flags),
        "failed to reset command buffer")
    recordCommandBuffer(commandBuffers[frame], imageIndex);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = STYPE(SUBMIT_INFO);

    VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[frame]};
    VkPipelineStageFlags waitStages[] = {
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffers[frame];

    VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[frame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    CHK(vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[frame]),
        "failed to submit draw command buffer")

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = STYPE(PRESENT_INFO_KHR);
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {swapChain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;

    presentInfo.pImageIndices = &imageIndex;

    VkResult presentResult = vkQueuePresentKHR(presentQueue, &presentInfo);
    if (presentResult == VK_ERROR_OUT_OF_DATE_KHR ||
        presentResult == VK_SUBOPTIMAL_KHR || framebufferResized) {
      framebufferResized = false;
      recreateSwapChain();
    } else if (presentResult != VK_SUCCESS) {
      throw std ::runtime_error("failed to present swap chain image");
    }

    ++frameIndex;
  }

  void updateUniformBuffer(uint32_t frame) {
    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(
                     currentTime - startTime)
                     .count();

    UniformBufferObject ubo{};
    ubo.time = time;

    ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f),
                            glm::vec3(0.0f, 0.0f, 1.0f));

    ubo.view =
        glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f),
                    glm::vec3(0.0f, 0.0f, 1.0f));

    float fovY = glm::radians(45.0f);
    float aspectRatio = swapChainExtent.width / (float)swapChainExtent.height;
    float zNear = 0.1f;
    float zFar = 10.0f;
    ubo.proj = glm::perspective(fovY, aspectRatio, zNear, zFar);
    ubo.proj[1][1] *= -1;

    memcpy(uniformBuffersMapped[frame], &ubo, sizeof(ubo));
  }

  void cleanupSwapChain() {
    for (auto framebuffer : swapChainFramebuffers) {
      vkDestroyFramebuffer(device, framebuffer, nullptr);
    }
    for (auto imageView : swapChainImageViews) {
      vkDestroyImageView(device, imageView, nullptr);
    }
    vkDestroySwapchainKHR(device, swapChain, nullptr);
  }

  void recreateSwapChain() {
    int width = 0;
    int height = 0;

    glfwGetFramebufferSize(window, &width, &height);
    while (width == 0 || height == 0) {
      glfwWaitEvents();
      glfwGetFramebufferSize(window, &width, &height);
    }

    vkDeviceWaitIdle(device);

    cleanupSwapChain();
    createSwapChain();
    createImageViews();
    createFramebuffers();
  }

  void cleanup() {
    cleanupSwapChain();

    vkDestroyPipeline(device, graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    vkDestroyRenderPass(device, renderPass, nullptr);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
      vkDestroyBuffer(device, uniformBuffers[i], nullptr);
      vkFreeMemory(device, uniformBuffersMemory[i], nullptr);
    }

    vkDestroySampler(device, textureSampler, nullptr);
    vkDestroyImageView(device, textureImageView, nullptr);

    vkDestroyImage(device, textureImage, nullptr);
    vkFreeMemory(device, textureImageMemory, nullptr);

    vkDestroyDescriptorPool(device, descriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);

    vkDestroyBuffer(device, vertexBuffer, nullptr);
    vkFreeMemory(device, vertexBufferMemory, nullptr);

    vkDestroyBuffer(device, indexBuffer, nullptr);
    vkFreeMemory(device, indexBufferMemory, nullptr);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
      vkDestroyFence(device, inFlightFences[i], nullptr);
      vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
      vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
    }

    vkDestroyCommandPool(device, commandPool, nullptr);

    vkDestroyDevice(device, nullptr);

    if (enableValidationLayers) {
      DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
    }

    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyInstance(instance, nullptr);

    glfwDestroyWindow(window);
    glfwTerminate();
  }

  static std::vector<char> readFile(const std::string &filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);
    if (!file.is_open()) {
      throw std::runtime_error(
          std::format("failed to open file '{}'", filename));
    }

    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();

    return buffer;
  }
};

int main() {
  MyApplication app;

  try {
    app.run();
  } catch (const std::exception &e) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
