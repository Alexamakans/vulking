#include "util.hpp"

#include <vulking/vulking.hpp>

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
}
