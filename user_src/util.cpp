#include "util.hpp"

GLFWwindow *createWindow() {
  glfwInit();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
  return CHK_GLFW(glfwCreateWindow(800, 600, "TestWindow", nullptr, nullptr),
                  "failed to create GLFW window");
}

vk::UniqueRenderPass createRenderPass(const Vulking::Context &ctx) {
  const auto info = Vulking::RenderPassInfo().Create(ctx.swapchain.imageFormat,
                                                     ctx.msaaSamples);
  return ctx.device->createRenderPassUnique(info.toCreateInfo());
}

vk::UniqueDescriptorSetLayout
createDescriptorSetLayout(const Vulking::Context &ctx) {
  vk::DescriptorSetLayoutBinding base{};
  base.setDescriptorCount(1).setPImmutableSamplers(nullptr);

  auto ubo = base;
  ubo.setBinding(0)
      .setDescriptorType(vk::DescriptorType::eUniformBuffer)
      .setStageFlags(vk::ShaderStageFlagBits::eVertex);

  auto sampler = base;
  sampler.setBinding(1)
      .setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
      .setStageFlags(vk::ShaderStageFlagBits::eFragment);

  std::array<vk::DescriptorSetLayoutBinding, 2> bindings{ubo, sampler};
  vk::DescriptorSetLayoutCreateInfo info{};
  info.setBindings(bindings);

  return ctx.device->createDescriptorSetLayoutUnique(info);
}

Shader loadShader(const Vulking::Context &ctx, const std::string &path,
                  const std::string &entrypoint, const char *name) {
  auto code = readFile(path);

  auto info = vk::ShaderModuleCreateInfo{}
                  .setCodeSize(code.size())
                  .setPCode(reinterpret_cast<const uint32_t *>(code.data()));

  auto module = ctx.device->createShaderModule(info);
  NAME_OBJECT(ctx.device, module, name);
  return {
      .module = module,
      .entrypoint = entrypoint,
  };
}

std::tuple<vk::UniquePipeline, vk::UniquePipelineLayout> createGraphicsPipeline(
    const Vulking::Context &ctx, const vk::UniqueRenderPass &renderPass,
    const std::map<vk::ShaderStageFlagBits, Shader> &shaders,
    const std::vector<vk::DescriptorSetLayout> &descriptorSetLayouts,
    const char *name) {
  std::vector<vk::PipelineShaderStageCreateInfo> shaderStageInfos{};
  for (auto &entry : shaders) {
    const auto &stage = entry.first;
    const auto &module = entry.second.module;
    const auto &entrypoint = entry.second.entrypoint;
    shaderStageInfos.push_back(vk::PipelineShaderStageCreateInfo{}
                                   .setStage(stage)
                                   .setModule(module)
                                   .setPName(entrypoint.c_str()));
  }

  auto bindingDescriptions = {Vulking::Mesh::Vertex::getBindingDescription()};
  auto attributeDescriptions =
      Vulking::Mesh::Vertex::getAttributeDescriptions();
  auto vertexInputInfo =
      vk::PipelineVertexInputStateCreateInfo{}
          .setVertexBindingDescriptions(bindingDescriptions)
          .setVertexAttributeDescriptions(attributeDescriptions);

  auto inputAssemblyInfo =
      vk::PipelineInputAssemblyStateCreateInfo{}
          .setTopology(vk::PrimitiveTopology::eTriangleList)
          .setPrimitiveRestartEnable(vk::False);

  auto viewportInfo =
      vk::PipelineViewportStateCreateInfo{}.setViewportCount(1).setScissorCount(
          1);

  auto rasterizationInfo = vk::PipelineRasterizationStateCreateInfo{}
                               .setDepthClampEnable(vk::False)
                               .setDepthBiasEnable(vk::False)
                               .setRasterizerDiscardEnable(vk::False)
                               .setPolygonMode(vk::PolygonMode::eFill)
                               .setCullMode(vk::CullModeFlagBits::eBack)
                               .setFrontFace(vk::FrontFace::eCounterClockwise)
                               .setLineWidth(1.0f);

  auto depthStencilInfo = vk::PipelineDepthStencilStateCreateInfo{}
                              .setDepthTestEnable(vk::True)
                              .setDepthWriteEnable(vk::True)
                              .setDepthCompareOp(vk::CompareOp::eLess)
                              .setDepthBoundsTestEnable(vk::False)
                              .setStencilTestEnable(vk::False);

  auto multisampleInfo = vk::PipelineMultisampleStateCreateInfo{}
                             .setSampleShadingEnable(vk::False)
                             .setRasterizationSamples(ctx.msaaSamples);

#define Color(x) vk::ColorComponentFlagBits::e##x
  auto colorBlendAttachment =
      vk::PipelineColorBlendAttachmentState{}
          .setColorWriteMask(Color(R) | Color(G) | Color(B) | Color(A))
          .setBlendEnable(vk::False);
#undef Color

  auto colorBlendInfo = vk::PipelineColorBlendStateCreateInfo{}
                            .setLogicOpEnable(vk::False)
                            .setLogicOp(vk::LogicOp::eCopy)
                            .setAttachments({colorBlendAttachment})
                            .setBlendConstants({0.0f, 0.0f, 0.0f, 0.0f});

  std::vector<vk::DynamicState> dynamicStates{
      vk::DynamicState::eViewport,
      vk::DynamicState::eScissor,
  };

  auto dynamicInfo =
      vk::PipelineDynamicStateCreateInfo{}.setDynamicStates(dynamicStates);

  auto layoutInfo =
      vk::PipelineLayoutCreateInfo{}.setSetLayouts(descriptorSetLayouts);

  auto layout = ctx.device->createPipelineLayoutUnique(layoutInfo);

  auto pipelineInfo = vk::GraphicsPipelineCreateInfo{}
                          .setStages(shaderStageInfos)
                          .setPVertexInputState(&vertexInputInfo)
                          .setPInputAssemblyState(&inputAssemblyInfo)
                          .setPViewportState(&viewportInfo)
                          .setPRasterizationState(&rasterizationInfo)
                          .setPMultisampleState(&multisampleInfo)
                          .setPDepthStencilState(&depthStencilInfo)
                          .setPColorBlendState(&colorBlendInfo)
                          .setPDynamicState(&dynamicInfo)
                          .setLayout(layout.get())
                          .setRenderPass(renderPass.get())
                          .setSubpass(0)
                          .setBasePipelineHandle(VK_NULL_HANDLE);

  auto pipeline =
      ctx.device->createGraphicsPipelineUnique(VK_NULL_HANDLE, pipelineInfo);
  switch (pipeline.result) {
  case vk::Result::eSuccess:
  case vk::Result::ePipelineCompileRequiredEXT:
    break;
  default:
    throw std::runtime_error(
        std::format("failed creating graphics pipeline: {}",
                    vk::to_string(pipeline.result)));
  }
  NAME_OBJECT(ctx.device, pipeline.value.get(), name);

  return std::make_tuple(std::move(pipeline.value), std::move(layout));
}

void Shader::destroy() {
  Vulking::Engine::ctx().device->destroyShaderModule(module);
  module = nullptr;
}
