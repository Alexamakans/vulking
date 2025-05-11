#pragma once

#include "../common.hpp"
#include "DescriptorSetLayout.hpp"
#include "Device.hpp"
#include "RenderPass.hpp"
#include "ShaderStageInfo.hpp"
#include <vulkan/vulkan_core.h>

namespace Vulking {
class GraphicsPipeline {
public:
  GraphicsPipeline(
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
      std::vector<DescriptorSetLayout> descriptorSetLayouts);

  void release();

  operator VkPipeline() const;
  VkPipelineLayout getLayout() const { return layout; }

private:
  const Device &device;
  const RenderPass &renderPass;
  VkPipelineLayout layout;
  VkPipeline pipeline;

  void
  init(std::vector<ShaderStageInfo> shaderStages,
       std::vector<VkVertexInputBindingDescription> bindingDescriptions,
       std::vector<VkVertexInputAttributeDescription> attributeDescriptions,
       VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateInfo,
       VkPipelineViewportStateCreateInfo viewportStateInfo,
       VkPipelineRasterizationStateCreateInfo rasterizationStateInfo,
       VkPipelineMultisampleStateCreateInfo multisampleStateInfo,
       VkPipelineDepthStencilStateCreateInfo depthStencilStateInfo,
       VkPipelineColorBlendStateCreateInfo colorBlendStateInfo,
       VkPipelineDynamicStateCreateInfo dynamicStateInfo,
       std::vector<DescriptorSetLayout> descriptorSetLayouts);

  VkPipelineVertexInputStateCreateInfo createVertexInputStateCreateInfo(
      std::vector<VkVertexInputBindingDescription> &bindingDescriptions,
      std::vector<VkVertexInputAttributeDescription> &attributeDescriptions);
};
} // namespace Vulking
