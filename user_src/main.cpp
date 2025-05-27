#include "util.hpp"
#include "vulking/Buffer.hpp"
#include "vulking/Image.hpp"

#include <GLFW/glfw3.h>
#include <chrono>
#include <ranges>
#include <vulking/vulking.hpp>

struct UBO {
  alignas(4) glm::float32 time;
  alignas(16) glm::mat4 model;
  alignas(16) glm::mat4 view;
  alignas(16) glm::mat4 proj;
};

void updateUBO(Vulking::Buffer<UBO> &buffer, uint32_t index);
void updateDescriptorSets(std::vector<vk::UniqueDescriptorSet> &sets,
                          std::vector<Vulking::Buffer<UBO>> &uboBuffers,
                          vk::ImageView textureImageView, vk::Sampler sampler);

int main() {
  auto window = createWindow();
  auto extensions = getGlfwRequiredInstanceExtensions();
  Vulking::Engine engine(window, "Game?", vk::makeApiVersion(0, 0, 0, 1),
                         extensions);

  std::map<vk::ShaderStageFlagBits, Shader> shaders = {
      {vk::ShaderStageFlagBits::eVertex,
       loadShader("assets/shaders/test.vert", "main", "vertex_shader")},
      {vk::ShaderStageFlagBits::eFragment,
       loadShader("assets/shaders/test.frag", "main", "fragment_shader")},
  };

  auto renderPass = createRenderPass();
  auto descriptorSetLayout = createDescriptorSetLayout();
  std::vector<vk::DescriptorSetLayout> descriptorSetLayouts{
      descriptorSetLayout.get()};
  auto [pipeline, pipelineLayout] = createGraphicsPipeline(
      renderPass, shaders, descriptorSetLayouts, "graphics_pipeline");

  engine.createFramebuffers(renderPass);

  auto mesh = Vulking::Mesh("assets/models/viking_room.obj", "viking_room");
  mesh.releaseCPUResources();

  const auto swapchainImageCount = engine.getSwapchainImageCount();

  auto descriptorPool = Vulking::createDescriptorPool(
      swapchainImageCount,
      {{vk::DescriptorType::eUniformBuffer, swapchainImageCount},
       {vk::DescriptorType::eCombinedImageSampler, swapchainImageCount}});

  auto uboBuffers = std::vector<Vulking::Buffer<UBO>>(swapchainImageCount);
  for (uint32_t i = 0; i < swapchainImageCount; i++) {
    uboBuffers.emplace_back(1, Vulking::BufferUsage::UNIFORM,
                            Vulking::BufferMemory::UNIFORM);
  }

  const std::vector<vk::DescriptorSetLayout> layouts(swapchainImageCount,
                                                     descriptorSetLayout.get());
  auto descriptorSets = Vulking::allocateDescriptorSet(descriptorPool, layouts);

  // this shouldn't be here start
  auto textureImage = Vulking::Image();
  // this shouldn't be here end

  while (true) {
    glfwPollEvents();
    auto ok = engine.beginRender();
    if (!ok) {
      continue;
    }
    auto [cmd, index] = ok.value();
    // draw frame start

    cmd.begin(vk::CommandBufferBeginInfo{});

    auto clearValues = std::array<vk::ClearValue, 2>{};
    clearValues[0].setColor(
        vk::ClearColorValue{}.setFloat32({0.0f, 0.0f, 0.0f, 0.0f}));
    clearValues[1].setDepthStencil({1.0f, 0});
    const auto renderPassBeginInfo =
        vk::RenderPassBeginInfo{}
            .setRenderPass(renderPass.get())
            .setFramebuffer(engine.getFramebuffer())
            .setRenderArea(vk::Rect2D{}.setExtent(engine.swapchainExtent))
            .setClearValues(clearValues);

    cmd.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);
    {
      cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline.get());

      const auto viewport = vk::Viewport{}
                                .setX(0.0f)
                                .setY(0.0f)
                                .setWidth((float)engine.swapchainExtent.width)
                                .setHeight((float)engine.swapchainExtent.height)
                                .setMinDepth(0.0f)
                                .setMaxDepth(1.0f);
      cmd.setViewport(0, 1, &viewport);

      const auto scissor =
          vk::Rect2D{}.setExtent(engine.swapchainExtent).setOffset({0, 0});
      cmd.setScissor(0, 1, &scissor);

      // binds mesh's vertex and index buffers
      mesh.bind(cmd);

      cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
                             pipelineLayout.get(), 0,
                             {descriptorSets[index].get()}, {});

      cmd.drawIndexed(mesh.getNumIndices(), 1, 0, 0, 0);
    }
    cmd.endRenderPass();
    cmd.end();

    // draw frame end
    engine.endRender({cmd});
  }
}

void updateUBO(Vulking::Buffer<UBO> &buffer, uint32_t index) {
  static auto startTime = std::chrono::high_resolution_clock::now();

  auto currentTime = std::chrono::high_resolution_clock::now();
  float time = std::chrono::duration<float, std::chrono::seconds::period>(
                   currentTime - startTime)
                   .count();

  UBO ubo{};
  ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f),
                          glm::vec3(0.0f, 0.0f, 1.0f));
  ubo.view =
      glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f),
                  glm::vec3(0.0f, 0.0f, 1.0f));
  ubo.proj =
      glm::perspective(glm::radians(45.0f),
                       Vulking::Engine::swapchainExtent.width /
                           (float)Vulking::Engine::swapchainExtent.height,
                       0.1f, 10.0f);
  ubo.proj[1][1] *= -1;

  buffer.set(&ubo, 1);
}

void updateDescriptorSets(std::vector<vk::UniqueDescriptorSet> &sets,
                          std::vector<Vulking::Buffer<UBO>> &uboBuffers,
                          vk::ImageView textureImageView, vk::Sampler sampler) {
  for (auto [i, set] : std::views::enumerate(sets)) {
    const auto bufferInfo = vk::DescriptorBufferInfo{}
                                .setBuffer(uboBuffers[i].getBuffer())
                                .setOffset(0)
                                .setRange(sizeof(UBO));

    const auto imageInfo =
        vk::DescriptorImageInfo{}
            .setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
            .setImageView(textureImageView)
            .setSampler(sampler);
  }
}
