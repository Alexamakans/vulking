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
  vkDestroyPipelineLayout(device, pipelineLayout, allocator);
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
  pipelineLayoutInfo.pSetLayouts =
      reinterpret_cast<VkDescriptorSetLayout *>(descriptorSetLayouts.data());

  throw std::runtime_error("WAOW it crash here goodnight");
  CHK(vkCreatePipelineLayout(device, &pipelineLayoutInfo, allocator,
                             &pipelineLayout),
      "failed to create pipeline layout");

  VkGraphicsPipelineCreateInfo graphicsPipelineInfo{};
  graphicsPipelineInfo.sType = STYPE(GRAPHICS_PIPELINE_CREATE_INFO);
  graphicsPipelineInfo.stageCount = shaderStages.size();
  graphicsPipelineInfo.pStages =
      reinterpret_cast<VkPipelineShaderStageCreateInfo *>(shaderStages.data());
  graphicsPipelineInfo.pVertexInputState = &vertexInputStateInfo;
  graphicsPipelineInfo.pInputAssemblyState = &inputAssemblyStateInfo;
  graphicsPipelineInfo.pViewportState = &viewportStateInfo;
  graphicsPipelineInfo.pRasterizationState = &rasterizationStateInfo;
  graphicsPipelineInfo.pMultisampleState = &multisampleStateInfo;
  graphicsPipelineInfo.pDepthStencilState = &depthStencilStateInfo;
  graphicsPipelineInfo.pColorBlendState = &colorBlendStateInfo;
  graphicsPipelineInfo.pDynamicState = &dynamicStateInfo;
  graphicsPipelineInfo.layout = pipelineLayout;
  graphicsPipelineInfo.renderPass = renderPass;
  graphicsPipelineInfo.subpass = 0;
  graphicsPipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

  CHK(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1,
                                &graphicsPipelineInfo, allocator, &pipeline),
      "failed to create graphics pipeline");
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
