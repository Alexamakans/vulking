#pragma once

#include <map>
#include <tuple>
#include <vulking/vulking.hpp>

GLFWwindow *createWindow();

struct Shader {
  vk::ShaderModule module;
  std::string entrypoint;

  void destroy();
};

vk::UniqueRenderPass createRenderPass(const Vulking::Context &ctx);

vk::UniqueDescriptorSetLayout
createDescriptorSetLayout(const Vulking::Context &ctx);

Shader loadShader(const Vulking::Context &ctx, const std::string &path,
                  const std::string &entrypoint = "main",
                  const char *name = "unnamed");

std::tuple<vk::UniquePipeline, vk::UniquePipelineLayout> createGraphicsPipeline(
    const Vulking::Context &ctx, const vk::UniqueRenderPass &renderPass,
    const std::map<vk::ShaderStageFlagBits, Shader> &shaders,
    const std::vector<vk::DescriptorSetLayout> &descriptorSetLayouts,
    const char *name = "unnamed");
