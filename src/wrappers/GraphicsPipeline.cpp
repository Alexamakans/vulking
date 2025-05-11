#include "GraphicsPipeline.hpp"
#include <vulkan/vulkan_core.h>

Vulking::GraphicsPipeline::GraphicsPipeline(
    const Device &device, const RenderPass &renderPass,
    std::vector<ShaderStageInfo> shaderStages,
    std::vector<VkVertexInputBindingDescription> bindingDescriptions,
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions,
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateInfo,
    VkPipelineViewportStateCreateInfo viewportStateInfo,
    VkPipelineRasterizationStateCreateInfo rasterizationStateInfo,
    VkPipelineMultisampleStateCreateInfo multisampleStateInfo,
    VkPipelineDepthStencilStateCreateInfo depthStencilStateInfo,
    VkPipelineColorBlendStateCreateInfo colorBlendStateInfo,
    VkPipelineDynamicStateCreateInfo dynamicStateInfo,
    std::vector<DescriptorSetLayout> descriptorSetLayouts)
    : device(device), renderPass(renderPass) {
  init(shaderStages, bindingDescriptions, attributeDescriptions,
       inputAssemblyStateInfo, viewportStateInfo, rasterizationStateInfo,
       multisampleStateInfo, depthStencilStateInfo, colorBlendStateInfo,
       dynamicStateInfo, descriptorSetLayouts);
}

void Vulking::GraphicsPipeline::release() {
  vkDestroyPipelineLayout(device, layout, allocator);
  vkDestroyPipeline(device, pipeline, allocator);
};

Vulking::GraphicsPipeline::operator VkPipeline() const { return pipeline; }

void Vulking::GraphicsPipeline::init(
    std::vector<ShaderStageInfo> shaderStages,
    std::vector<VkVertexInputBindingDescription> bindingDescriptions,
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions,
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateInfo,
    VkPipelineViewportStateCreateInfo viewportStateInfo,
    VkPipelineRasterizationStateCreateInfo rasterizationStateInfo,
    VkPipelineMultisampleStateCreateInfo multisampleStateInfo,
    VkPipelineDepthStencilStateCreateInfo depthStencilStateInfo,
    VkPipelineColorBlendStateCreateInfo colorBlendStateInfo,
    VkPipelineDynamicStateCreateInfo dynamicStateInfo,
    std::vector<DescriptorSetLayout> descriptorSetLayouts) {

  auto vertexInputStateInfo = createVertexInputStateCreateInfo(
      bindingDescriptions, attributeDescriptions);

  VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
  pipelineLayoutInfo.sType = STYPE(PIPELINE_LAYOUT_CREATE_INFO);
  pipelineLayoutInfo.setLayoutCount =
      static_cast<uint32_t>(descriptorSetLayouts.size());
  std::vector<VkDescriptorSetLayout> rawSetLayouts;
  for (auto &setLayout : descriptorSetLayouts) {
    rawSetLayouts.push_back(setLayout);
  }
  pipelineLayoutInfo.pSetLayouts = rawSetLayouts.data();

  CHK(vkCreatePipelineLayout(device, &pipelineLayoutInfo, allocator,
                             &layout),
      "failed to create pipeline layout");

  VkGraphicsPipelineCreateInfo graphicsPipelineInfo{};
  graphicsPipelineInfo.sType = STYPE(GRAPHICS_PIPELINE_CREATE_INFO);
  graphicsPipelineInfo.stageCount = shaderStages.size();
  std::vector<VkPipelineShaderStageCreateInfo> rawShaderStages;
  for (auto &shaderStage : shaderStages) {
    rawShaderStages.push_back(shaderStage);
  }
  graphicsPipelineInfo.pStages = rawShaderStages.data();
  graphicsPipelineInfo.pVertexInputState = &vertexInputStateInfo;
  graphicsPipelineInfo.pInputAssemblyState = &inputAssemblyStateInfo;
  graphicsPipelineInfo.pViewportState = &viewportStateInfo;
  graphicsPipelineInfo.pRasterizationState = &rasterizationStateInfo;
  graphicsPipelineInfo.pMultisampleState = &multisampleStateInfo;
  graphicsPipelineInfo.pDepthStencilState = &depthStencilStateInfo;
  graphicsPipelineInfo.pColorBlendState = &colorBlendStateInfo;
  graphicsPipelineInfo.pDynamicState = &dynamicStateInfo;
  graphicsPipelineInfo.layout = layout;
  graphicsPipelineInfo.renderPass = renderPass;
  graphicsPipelineInfo.subpass = 0;
  graphicsPipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

  CHK(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1,
                                &graphicsPipelineInfo, allocator, &pipeline),
      "failed to create graphics pipeline");

  for (auto &shaderStage : shaderStages) {
    shaderStage.release();
  }
}

VkPipelineVertexInputStateCreateInfo
Vulking::GraphicsPipeline::createVertexInputStateCreateInfo(
    std::vector<VkVertexInputBindingDescription> &bindingDescriptions,
    std::vector<VkVertexInputAttributeDescription> &attributeDescriptions) {
  VkPipelineVertexInputStateCreateInfo info{};
  info.sType = STYPE(PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO);

  info.vertexBindingDescriptionCount =
      static_cast<uint32_t>(bindingDescriptions.size());
  info.pVertexBindingDescriptions = bindingDescriptions.data();

  info.vertexAttributeDescriptionCount =
      static_cast<uint32_t>(attributeDescriptions.size());
  info.pVertexAttributeDescriptions = attributeDescriptions.data();

  return info;
}
