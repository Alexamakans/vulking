#include "ShaderStageInfo.hpp"

Vulking::ShaderStageInfo::ShaderStageInfo(const Device &device,
                                          const std::string &path,
                                          VkShaderStageFlagBits stage)
    : device(device) {
  auto code = readFile(path);
  info.sType = STYPE(PIPELINE_SHADER_STAGE_CREATE_INFO);
  info.stage = stage;
  info.module = createShaderModule(code);
  info.pName = "main";
}

void Vulking::ShaderStageInfo::release() {
  std::cout << "Releasing shader stage: " << info.stage << std::endl;
  vkDestroyShaderModule(device, info.module, allocator);
}

Vulking::ShaderStageInfo::operator VkPipelineShaderStageCreateInfo() const {
  return info;
}

VkShaderModule
Vulking::ShaderStageInfo::createShaderModule(const std::vector<char> &code) {
  VkShaderModuleCreateInfo moduleInfo{};
  moduleInfo.sType = STYPE(SHADER_MODULE_CREATE_INFO);
  moduleInfo.codeSize = code.size();
  moduleInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());
  VkShaderModule module;
  CHK(vkCreateShaderModule(device, &moduleInfo, allocator, &module),
      "failed to create shader module");
  return module;
}
