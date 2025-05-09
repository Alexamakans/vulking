#pragma once

#include "../common.hpp"
#include "Device.hpp"

namespace Vulking {
class ShaderStageInfo {
public:
  ShaderStageInfo(const Device &device, const std::string &path,
                  VkShaderStageFlagBits stage);

  void release();

  operator VkPipelineShaderStageCreateInfo() const;

private:
  const Device &device;

  VkPipelineShaderStageCreateInfo info{};

  VkShaderModule createShaderModule(const std::vector<char> &code);
};

} // namespace Vulking
