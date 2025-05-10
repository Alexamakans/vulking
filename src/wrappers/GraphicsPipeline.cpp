#include "GraphicsPipeline.hpp"
#include <vulkan/vulkan_core.h>

Vulking::GraphicsPipeline::GraphicsPipeline(
    const Device &device, std::vector<ShaderStageInfo> &shaderStages,
    std::vector<VkVertexInputBindingDescription> &bindingDescriptions,
    std::vector<VkVertexInputAttributeDescription> &attributeDescriptions,
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateInfo,
    VkPipelineViewportStateCreateInfo viewportStateInfo,
    VkPipelineRasterizationStateCreateInfo rasterizationStateInfo,
    VkPipelineMultisampleStateCreateInfo multisampleStateInfo,
    VkPipelineDepthStencilStateCreateInfo depthStencilStateInfo,
    VkPipelineColorBlendStateCreateInfo colorBlendStateInfo,
    VkPipelineDynamicStateCreateInfo dynamicStateInfo,
    std::vector<DescriptorSetLayout> &descriptorSetLayouts)
    : device(device),
      pipeline(createPipeline(
          shaderStages, bindingDescriptions, attributeDescriptions,
          inputAssemblyStateInfo, viewportStateInfo, rasterizationStateInfo,
          multisampleStateInfo, depthStencilStateInfo, colorBlendStateInfo,
          dynamicStateInfo, descriptorSetLayouts)) {}

void Vulking::GraphicsPipeline::release() {
  vkDestroyPipelineLayout(device, pipelineLayout, allocator);
  vkDestroyPipeline(device, pipeline, allocator);
};

Vulking::GraphicsPipeline::operator VkPipeline() const { return pipeline; }

VkPipeline Vulking::GraphicsPipeline::createPipeline(
    std::vector<ShaderStageInfo> &shaderStages,
    std::vector<VkVertexInputBindingDescription> &bindingDescriptions,
    std::vector<VkVertexInputAttributeDescription> &attributeDescriptions,
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateInfo,
    VkPipelineViewportStateCreateInfo viewportStateInfo,
    VkPipelineRasterizationStateCreateInfo rasterizationStateInfo,
    VkPipelineMultisampleStateCreateInfo multisampleStateInfo,
    VkPipelineDepthStencilStateCreateInfo depthStencilStateInfo,
    VkPipelineColorBlendStateCreateInfo colorBlendStateInfo,
    VkPipelineDynamicStateCreateInfo dynamicStateInfo,
    std::vector<DescriptorSetLayout> &descriptorSetLayouts) {

  VkPipelineShaderStageCreateInfo *pShaderStages =
      reinterpret_cast<VkPipelineShaderStageCreateInfo *>(shaderStages.data());
  auto stageCount = shaderStages.size();

  auto vertexInputStateCreateInfo = createVertexInputStateCreateInfo(
      bindingDescriptions, attributeDescriptions);

  VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
  pipelineLayoutInfo.sType = STYPE(PIPELINE_LAYOUT_CREATE_INFO);
  pipelineLayoutInfo.setLayoutCount =
      static_cast<uint32_t>(descriptorSetLayouts.size());
  pipelineLayoutInfo.pSetLayouts =
      reinterpret_cast<VkDescriptorSetLayout *>(descriptorSetLayouts.data());

  CHK(vkCreatePipelineLayout(device, &pipelineLayoutInfo, allocator,
                             &pipelineLayout),
      "failed to create pipeline layout");

  VkGraphicsPipelineCreateInfo graphicsPipelineInfo{};
  graphicsPipelineInfo.sType = STYPE(
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
