#include "common.hpp"
#include "devices/GPU.hpp"
#include "helpers/VulkingUtil.hpp"
#include "resources/Vertex.hpp"
#include "wrappers/DebugMessenger.hpp"
#include "wrappers/DescriptorSetLayout.hpp"
#include "wrappers/GraphicsPipeline.hpp"
#include "wrappers/Instance.hpp"
#include "wrappers/RenderPass.hpp"
#include "wrappers/Surface.hpp"
#include "wrappers/SwapChain.hpp"
#include <vulkan/vulkan_core.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

const int WIDTH = 800;
const int HEIGHT = 600;

VkDescriptorSetLayoutBinding uboLayoutBinding = {
    .binding = 0,
    .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
    .descriptorCount = 1,
    .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
    .pImmutableSamplers = nullptr,
};
VkDescriptorSetLayoutBinding samplerLayoutBinding = {
    .binding = 1,
    .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
    .descriptorCount = 1,
    .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
    .pImmutableSamplers = nullptr,
};

VkPipelineInputAssemblyStateCreateInfo createInputAssemblyStateInfo() {
  return {
      .sType = STYPE(PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO),
      .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
      .primitiveRestartEnable = VK_FALSE,
  };
}
VkPipelineViewportStateCreateInfo createViewportStateInfo() {
  return {
      .sType = STYPE(PIPELINE_VIEWPORT_STATE_CREATE_INFO),
      .viewportCount = 1,
      .scissorCount = 1,
  };
}
VkPipelineRasterizationStateCreateInfo createRasterizationStateInfo() {
  return {
      .sType = STYPE(PIPELINE_RASTERIZATION_STATE_CREATE_INFO),
      .depthClampEnable = VK_FALSE,
      .rasterizerDiscardEnable = VK_FALSE,
      .polygonMode = VK_POLYGON_MODE_FILL,
      .cullMode = VK_CULL_MODE_BACK_BIT,
      .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
      .depthBiasEnable = VK_FALSE,
      .lineWidth = 1.0f,
  };
}
VkPipelineMultisampleStateCreateInfo
createMultisampleStateInfo(VkSampleCountFlagBits msaaSamples) {
  return {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
      .rasterizationSamples = msaaSamples,
      .sampleShadingEnable = VK_FALSE,
  };
}
VkPipelineDepthStencilStateCreateInfo createDepthStencilStateInfo() {
  return {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
      .depthTestEnable = VK_TRUE,
      .depthWriteEnable = VK_TRUE,
      .depthCompareOp = VK_COMPARE_OP_LESS,
      .depthBoundsTestEnable = VK_FALSE,
      .stencilTestEnable = VK_FALSE,
  };
}
VkPipelineColorBlendAttachmentState createColorBlendAttachmentState() {
  return {
      .blendEnable = VK_FALSE,
      .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
  };
}
VkPipelineColorBlendStateCreateInfo createColorBlendStateInfo(
    VkPipelineColorBlendAttachmentState &colorBlendAttachment) {
  return {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
      .logicOpEnable = VK_FALSE,
      .logicOp = VK_LOGIC_OP_COPY,
      .attachmentCount = 1,
      .pAttachments = &colorBlendAttachment,
      .blendConstants = {0.0f, 0.0f, 0.0f, 0.0f},
  };
}

std::vector<VkDynamicState> dynamicStates = {VK_DYNAMIC_STATE_VIEWPORT,
                                             VK_DYNAMIC_STATE_SCISSOR};
VkPipelineDynamicStateCreateInfo createDynamicStateInfo() {
  return {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
      .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
      .pDynamicStates = dynamicStates.data(),
  };
}

auto colorBlendAttachment = createColorBlendAttachmentState();

class MyApplication {
public:
  GLFWwindow *window;
  Vulking::Instance instance;
  Vulking::DebugMessenger debugMessenger;
  Vulking::Surface surface;
  Vulking::GPU gpu;
  Vulking::RenderPass renderPass;
  Vulking::SwapChain swapChain;
  Vulking::DescriptorSetLayout descriptorSetLayout;
  Vulking::GraphicsPipeline graphicsPipeline;

  MyApplication()
      : window(createWindow()), instance(getRequiredExtensions(), true),
        debugMessenger(instance),
        surface(instance, createSurface(instance, window)),
        gpu(instance, surface),
        renderPass(gpu.device, VK_FORMAT_R32G32B32_SFLOAT,
                   VulkingUtil::findDepthFormat(gpu.physicalDevice),
                   gpu.physicalDevice.getMsaaSamples()),
        swapChain(gpu.physicalDevice, gpu.device, surface, gpu.getRenderPass()),
        descriptorSetLayout(gpu.device,
                            {uboLayoutBinding, samplerLayoutBinding}),
        graphicsPipeline(
            gpu.device, renderPass,
            {{Vulking::ShaderStageInfo(gpu.device, "assets/shaders/test.vert",
                                       VK_SHADER_STAGE_VERTEX_BIT),
              Vulking::ShaderStageInfo(gpu.device, "assets/shaders/test.frag",
                                       VK_SHADER_STAGE_FRAGMENT_BIT)}},
            {Vulking::Vertex::getBindingDescription()},
            Vulking::Vertex::getAttributeDescriptions(),
            createInputAssemblyStateInfo(), createViewportStateInfo(),
            createRasterizationStateInfo(),
            createMultisampleStateInfo(gpu.physicalDevice.getMsaaSamples()),
            createDepthStencilStateInfo(),
            createColorBlendStateInfo(colorBlendAttachment),
            createDynamicStateInfo(), {descriptorSetLayout}) {}

  void run() { release(); }

private:
  void release() {
    swapChain.release();
    renderPass.release();
    gpu.release();
    surface.release();
    debugMessenger.release();
    instance.release();
    glfwDestroyWindow(window);
    glfwTerminate();
  }

  GLFWwindow *createWindow() {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    GLFWwindow *window =
        glfwCreateWindow(WIDTH, HEIGHT, "Vulking", nullptr, nullptr);
    if (!window) {
      throw std::runtime_error("glfw window creation failed");
    }

    glfwSetWindowUserPointer(window, this);
    // glfwSetFramebufferSizeCallback(window, onFramebufferResize);
    return window;
  }

  std::vector<const char *> getRequiredExtensions() const {
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

  VkSurfaceKHR createSurface(const Vulking::Instance &instance,
                             GLFWwindow *window) {
    VkSurfaceKHR vkSurface;
    CHK(glfwCreateWindowSurface(instance, window, allocator, &vkSurface),
        "failed to create surface");
    return vkSurface;
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
