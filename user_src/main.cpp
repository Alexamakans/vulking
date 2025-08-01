#include "util.hpp"
#include "vulking/Buffer.hpp"
#include "vulking/Image.hpp"

#include <chrono>
#include <ranges>
#include <vulking/vulking.hpp>

struct UBO {
  alignas(4) glm::float32 time;
  alignas(16) glm::mat4 model;
  alignas(16) glm::mat4 view;
  alignas(16) glm::mat4 proj;
};

void updateUBO(const Vulking::Context &ctx, const Vulking::Buffer<UBO> &buffer);
void updateDescriptorSets(const Vulking::Context &ctx,
                          const std::vector<vk::UniqueDescriptorSet> &sets,
                          const std::vector<Vulking::Buffer<UBO>> &uboBuffers,
                          const vk::ImageView &textureImageView,
                          const vk::Sampler &sampler);

int main() {
  auto window = createWindow();
  auto extensions = getGlfwRequiredInstanceExtensions();
  Vulking::Engine engine(window, "Game?", vk::makeApiVersion(0, 0, 0, 1),
                         extensions);

  auto &ctx = engine.getContext();

  std::map<vk::ShaderStageFlagBits, Shader> shaders = {
      {vk::ShaderStageFlagBits::eVertex,
       loadShader(ctx, "assets/shaders/test.vert.spv", "main",
                  "vertex_shader")},
      {vk::ShaderStageFlagBits::eFragment,
       loadShader(ctx, "assets/shaders/test.frag.spv", "main",
                  "fragment_shader")},
  };

  auto renderPass = createRenderPass(ctx);
  auto descriptorSetLayout = createDescriptorSetLayout(ctx);
  std::vector<vk::DescriptorSetLayout> descriptorSetLayouts{
      descriptorSetLayout.get()};
  auto [pipeline, pipelineLayout] = createGraphicsPipeline(
      ctx, renderPass, shaders, descriptorSetLayouts, "graphics_pipeline");

  for (auto &[_, shader] : shaders) {
    shader.destroy();
  }

  ctx.swapchain.createFramebuffers(renderPass.get());

  const auto swapchainImageCount = ctx.swapchain.imageCount;
  auto descriptorPool = Vulking::createDescriptorPool(
      swapchainImageCount,
      {{vk::DescriptorType::eUniformBuffer, swapchainImageCount},
       {vk::DescriptorType::eCombinedImageSampler, swapchainImageCount}});

  std::vector<Vulking::Buffer<UBO>> uboBuffers;
  for (uint32_t i = 0; i < swapchainImageCount; i++) {
    uboBuffers.emplace_back(UBO{}, Vulking::BufferUsage::UNIFORM,
                            Vulking::BufferMemory::UNIFORM,
                            std::format("ubo_buffer_{}", i).c_str());
    uboBuffers[i].map();
  }

  const std::vector<vk::DescriptorSetLayout> layouts(swapchainImageCount,
                                                     descriptorSetLayout.get());
  auto descriptorSets = Vulking::allocateDescriptorSet(descriptorPool, layouts);

  // this shouldn't be here start
  auto mesh = Vulking::Mesh("assets/models/viking_room.obj", "viking_room");
  mesh.releaseCPUResources();

  auto textureImage =
      Vulking::Image("assets/textures/viking_room.png", ctx.msaaSamples,
                     vk::Format::eR8G8B8A8Srgb, "viking_room_texture");
  auto textureImageView = engine.getContext().createImageViewUnique(
      textureImage.image.get(), vk::Format::eR8G8B8A8Srgb,
      vk::ImageAspectFlagBits::eColor, textureImage.getMipLevels());
  auto textureSampler = Vulking::createSampler();

  updateDescriptorSets(ctx, descriptorSets, uboBuffers, textureImageView.get(),
                       textureSampler.get());
  // this shouldn't be here end

  while (!glfwWindowShouldClose(window)) {
    LOG_DEBUG("polling events");
    glfwPollEvents();
    LOG_DEBUG("beginning render");
    auto ok = ctx.beginRender();
    if (!ok) {
      LOG_DEBUG("beginRender returned false, skipped frame");
      continue;
    }
    auto [cmd, index] = ok.value();
    // draw frame start

    cmd.begin(vk::CommandBufferBeginInfo{});
    updateUBO(ctx, uboBuffers[ctx.swapchain.getCurrentResourceIndex()]);

    auto clearValues = std::array<vk::ClearValue, 2>{};
    clearValues[0].setColor(
        vk::ClearColorValue{}.setFloat32({0.0f, 0.0f, 0.0f, 0.0f}));
    clearValues[1].setDepthStencil({1.0f, 0});
    const auto renderPassBeginInfo =
        vk::RenderPassBeginInfo{}
            .setRenderPass(renderPass.get())
            .setFramebuffer(ctx.swapchain.getFramebuffer())
            .setRenderArea(vk::Rect2D{}.setExtent(ctx.swapchain.extent))
            .setClearValues(clearValues);

    cmd.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);
    {
      cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline.get());

      const auto viewport = vk::Viewport{}
                                .setX(0.0f)
                                .setY(0.0f)
                                .setWidth((float)ctx.swapchain.extent.width)
                                .setHeight((float)ctx.swapchain.extent.height)
                                .setMinDepth(0.0f)
                                .setMaxDepth(1.0f);
      cmd.setViewport(0, 1, &viewport);

      const auto scissor =
          vk::Rect2D{}.setExtent(ctx.swapchain.extent).setOffset({0, 0});
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
    LOG_DEBUG("ending render");
    ctx.endRender({cmd});
  }

  ctx.device->waitIdle();
}

void updateUBO(const Vulking::Context &ctx,
               const Vulking::Buffer<UBO> &buffer) {
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
  ubo.proj = glm::perspective(glm::radians(45.0f),
                              ctx.swapchain.extent.width /
                                  (float)ctx.swapchain.extent.height,
                              0.1f, 10.0f);
  ubo.proj[1][1] *= -1;

  buffer.set(ubo);
}

void updateDescriptorSets(const Vulking::Context &ctx,
                          const std::vector<vk::UniqueDescriptorSet> &sets,
                          const std::vector<Vulking::Buffer<UBO>> &uboBuffers,
                          const vk::ImageView &textureImageView,
                          const vk::Sampler &sampler) {
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

    std::array<vk::WriteDescriptorSet, 2> writes{};

    // image must be in the correct layout here

    writes[0]
        .setDstSet(set.get())
        .setDstBinding(0)
        .setDstArrayElement(0)
        .setDescriptorType(vk::DescriptorType::eUniformBuffer)
        .setBufferInfo(bufferInfo);

    writes[1]
        .setDstSet(set.get())
        .setDstBinding(1)
        .setDstArrayElement(0)
        .setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
        .setImageInfo(imageInfo);

    ctx.device->updateDescriptorSets(writes, {});
  }
}
