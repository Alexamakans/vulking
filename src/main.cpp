#include "common.hpp"
#include "devices/GPU.hpp"
#include "helpers/VulkingUtil.hpp"
#include "resources/Model.hpp"
#include "resources/Texture.hpp"
#include "resources/UniformBuffer.hpp"
#include "resources/Vertex.hpp"
#include "wrappers/DebugMessenger.hpp"
#include "wrappers/DescriptorSetLayout.hpp"
#include "wrappers/GraphicsPipeline.hpp"
#include "wrappers/Instance.hpp"
#include "wrappers/RenderPass.hpp"
#include "wrappers/Surface.hpp"
#include "wrappers/SwapChain.hpp"
#include <chrono>
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
    .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
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

struct UBO {
  alignas(4) glm::float32 time;
  alignas(16) glm::mat4 model;
  alignas(16) glm::mat4 view;
  alignas(16) glm::mat4 proj;
};

class MyApplication {
public:
  GLFWwindow *window;
  Vulking::Instance instance;
  Vulking::DebugMessenger debugMessenger;
  Vulking::Surface surface;
  Vulking::GPU gpu;
  Vulking::SwapChain swapChain;
  Vulking::DescriptorSetLayout descriptorSetLayout;
  Vulking::GraphicsPipeline graphicsPipeline;

  std::vector<Vulking::UniformBuffer<UBO>> uniformBuffers;
  std::vector<VkCommandBuffer> commandBuffers;
  std::vector<VkDescriptorSet> descriptorSets;

  std::vector<VkFence> inFlightFences;
  std::vector<VkSemaphore> imageAvailableSemaphores;
  std::vector<VkSemaphore> renderFinishedSemaphores;

  Vulking::Texture texture;
  Vulking::Model model;

  int currentFrame = 0;
  bool framebufferResized = false;

  MyApplication()
      : window(createWindow()), instance(getRequiredExtensions(), true),
        debugMessenger(instance),
        surface(instance, createSurface(instance, window)),
        gpu(instance, surface),
        swapChain(gpu.physicalDevice, gpu.device, surface, gpu.getRenderPass()),
        descriptorSetLayout(gpu.device,
                            {uboLayoutBinding, samplerLayoutBinding}),
        graphicsPipeline(
            gpu.device, gpu.getRenderPass(),
            {{Vulking::ShaderStageInfo(gpu.device,
                                       "assets/shaders/test.vert.spv",
                                       VK_SHADER_STAGE_VERTEX_BIT),
              Vulking::ShaderStageInfo(gpu.device,
                                       "assets/shaders/test.frag.spv",
                                       VK_SHADER_STAGE_FRAGMENT_BIT)}},
            {Vulking::Vertex::getBindingDescription()},
            Vulking::Vertex::getAttributeDescriptions(),
            createInputAssemblyStateInfo(), createViewportStateInfo(),
            createRasterizationStateInfo(),
            createMultisampleStateInfo(gpu.physicalDevice.getMsaaSamples()),
            createDepthStencilStateInfo(),
            createColorBlendStateInfo(colorBlendAttachment),
            createDynamicStateInfo(), {descriptorSetLayout}),
        texture(gpu.physicalDevice, gpu.device, gpu.getCommandPool(),
                gpu.getGraphicsQueue(), "assets/textures/viking_room.png"),
        model(gpu.physicalDevice, gpu.device, gpu.getCommandPool(),
              gpu.getGraphicsQueue(), "assets/models/viking_room.obj") {}

  static void onFramebufferResize(GLFWwindow *window, int width, int height) {
    auto app =
        reinterpret_cast<MyApplication *>(glfwGetWindowUserPointer(window));
    app->framebufferResized = true;
  }

  void run() {
    init();
    mainLoop();
    release();
  }

private:
  void init() {
    uint32_t maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    for (uint32_t i = 0; i < maxSets; i++) {
      uniformBuffers.push_back(
          Vulking::UniformBuffer<UBO>(gpu.physicalDevice, gpu.device));
    }

    std::vector<VkDescriptorPoolSize> poolSizes{
        {
            .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = maxSets,
        },
        {
            .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = maxSets,
        },
    };

    gpu.setDescriptorPool(poolSizes, maxSets);
    descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
    auto writeSets = gpu.getDescriptorPool().allocateSets(
        descriptorSetLayout, maxSets, descriptorSets);
    for (size_t i = 0; i < writeSets.size(); i++) {
      VkDescriptorBufferInfo bufferInfo{};
      bufferInfo.buffer = uniformBuffers[i];
      bufferInfo.offset = 0;
      bufferInfo.range = sizeof(UBO);

      VkDescriptorImageInfo imageInfo{};
      imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
      imageInfo.imageView = texture.getView();
      imageInfo.sampler = texture.getSampler();

      auto &writes = writeSets[i];
      writes[0].dstBinding = 0;
      writes[0].dstArrayElement = 0;
      writes[0].descriptorType = poolSizes[0].type;
      writes[0].descriptorCount = 1;
      writes[0].pBufferInfo = &bufferInfo;

      writes[1].dstBinding = 1;
      writes[1].dstArrayElement = 0;
      writes[1].descriptorType = poolSizes[1].type;
      writes[1].descriptorCount = 1;
      writes[1].pImageInfo = &imageInfo;

      vkUpdateDescriptorSets(gpu.device, static_cast<uint32_t>(writes.size()),
                             writes.data(), 0, nullptr);
    }

    commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    gpu.getCommandPool().allocateBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                                        commandBuffers.size(),
                                        commandBuffers.data());

    createSyncObjects();
  }

  void mainLoop() {
    while (!glfwWindowShouldClose(window)) {
      glfwPollEvents();
      drawFrame();
    }
    vkDeviceWaitIdle(gpu.device);
  }

  void createSyncObjects() {
    imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
      if (vkCreateSemaphore(gpu.device, &semaphoreInfo, allocator,
                            &imageAvailableSemaphores[i]) != VK_SUCCESS ||
          vkCreateSemaphore(gpu.device, &semaphoreInfo, allocator,
                            &renderFinishedSemaphores[i]) != VK_SUCCESS ||
          vkCreateFence(gpu.device, &fenceInfo, allocator,
                        &inFlightFences[i]) != VK_SUCCESS) {
        throw std::runtime_error(
            "failed to create synchronization objects for a frame!");
      }
      NAME_OBJECT(gpu.device, VK_OBJECT_TYPE_SEMAPHORE,
                  imageAvailableSemaphores[i],
                  std::format("image_available_semaphore_{}", i).c_str());
      NAME_OBJECT(gpu.device, VK_OBJECT_TYPE_SEMAPHORE,
                  renderFinishedSemaphores[i],
                  std::format("render_finished_semaphore_{}", i).c_str());
      NAME_OBJECT(gpu.device, VK_OBJECT_TYPE_FENCE, inFlightFences[i],
                  std::format("in_flight_fence_{}", i).c_str());
    }
  }

  void updateUniformBuffer(uint32_t currentImage) {
    static auto startTime = std::chrono::high_resolution_clock::now();
    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(
                     currentTime - startTime)
                     .count();

    UBO ubo{};
    ubo.time = time;
    ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f),
                            glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.view =
        glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f),
                    glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.proj = glm::perspective(glm::radians(45.0f),
                                swapChain.getExtent().width /
                                    (float)swapChain.getExtent().height,
                                0.1f, 10.0f);
    ubo.proj[1][1] *= -1;

    uniformBuffers[currentImage].set(&ubo);
  }

  void drawFrame() {
    vkWaitForFences(gpu.device, 1, &inFlightFences[currentFrame], VK_TRUE,
                    UINT64_MAX);

    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(
        gpu.device, swapChain, UINT64_MAX,
        imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
      swapChain.recreate();
      return;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
      throw std::runtime_error("failed to acquire swap chain image!");
    }

    updateUniformBuffer(currentFrame);

    vkResetFences(gpu.device, 1, &inFlightFences[currentFrame]);

    vkResetCommandBuffer(commandBuffers[currentFrame],
                         /*VkCommandBufferResetFlagBits*/ 0);
    recordCommandBuffer(commandBuffers[currentFrame], imageIndex);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
    VkPipelineStageFlags waitStages[] = {
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffers[currentFrame];

    VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(gpu.getGraphicsQueue(), 1, &submitInfo,
                      inFlightFences[currentFrame]) != VK_SUCCESS) {
      throw std::runtime_error("failed to submit draw command buffer!");
    }

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {swapChain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;

    presentInfo.pImageIndices = &imageIndex;

    result = vkQueuePresentKHR(gpu.getPresentQueue(), &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
        framebufferResized) {
      framebufferResized = false;
      swapChain.recreate();
    } else if (result != VK_SUCCESS) {
      throw std::runtime_error("failed to present swap chain image!");
    }

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
  }

  void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
      throw std::runtime_error("failed to begin recording command buffer!");
    }

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = gpu.getRenderPass();
    renderPassInfo.framebuffer = swapChain.getFramebuffer(imageIndex);
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = swapChain.getExtent();

    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
    clearValues[1].depthStencil = {1.0f, 0};

    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo,
                         VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                      graphicsPipeline);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)swapChain.getExtent().width;
    viewport.height = (float)swapChain.getExtent().height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = swapChain.getExtent();
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    model.bindBuffers(commandBuffer);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            graphicsPipeline.getLayout(), 0, 1,
                            &descriptorSets[currentFrame], 0, nullptr);

    vkCmdDrawIndexed(commandBuffer, model.getNumIndices(), 1, 0, 0, 0);

    vkCmdEndRenderPass(commandBuffer);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
      throw std::runtime_error("failed to record command buffer!");
    }
  }

  void release() {
    graphicsPipeline.release();
    model.release();
    texture.release();
    for (auto &uniformBuffer : uniformBuffers) {
      uniformBuffer.release();
    }
    for (auto &e : renderFinishedSemaphores) {
      vkDestroySemaphore(gpu.device, e, allocator);
    }
    for (auto &e : imageAvailableSemaphores) {
      vkDestroySemaphore(gpu.device, e, allocator);
    }
    for (auto &e : inFlightFences) {
      vkDestroyFence(gpu.device, e, allocator);
    }
    descriptorSetLayout.release();
    swapChain.release();
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
    glfwSetFramebufferSizeCallback(window, onFramebufferResize);
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
