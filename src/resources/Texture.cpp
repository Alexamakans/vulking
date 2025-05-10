#include "Texture.hpp"
#include "../helpers/VulkingUtil.hpp"
#include "StagingBuffer.hpp"
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_handles.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

Vulking::Texture::Texture(const PhysicalDevice &physicalDevice,
                          const Device &device, const CommandPool &commandPool,
                          const Queue &queue, const std::string &path)
    : physicalDevice(physicalDevice), device(device) {
  stbi_uc *pixels =
      stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);
  if (!pixels) {
    throw std::runtime_error(
        std::format("failed to load texture image: {}", path));
  }

  VkDeviceSize size = width * height * 4;
  mipLevels =
      1 + static_cast<uint32_t>(std::floor(std::log2(std::max(width, height))));
  VkFormat format = VK_FORMAT_R8G8B8A8_SRGB;

  StagingBuffer stagingBuffer(physicalDevice, device, pixels, size);

  VulkingUtil::createImage(
      physicalDevice, device, width, height, mipLevels, VK_SAMPLE_COUNT_1_BIT,
      format, VK_IMAGE_TILING_OPTIMAL,
      VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
          VK_IMAGE_USAGE_SAMPLED_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, image, memory);

  VulkingUtil::transitionImageLayout(
      device, commandPool, queue, image, format, VK_IMAGE_LAYOUT_UNDEFINED,
      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipLevels);
  VulkingUtil::copyBufferToImage(device, commandPool, queue, stagingBuffer,
                                 image, width, height);

  VulkingUtil::generateMipmaps(physicalDevice, device, commandPool, queue,
                               image, format, width, height, mipLevels);

  view = VulkingUtil::createImageView(device, image, format,
                                      VK_IMAGE_ASPECT_COLOR_BIT, mipLevels);

  createSampler(physicalDevice, device);
}

void Vulking::Texture::createSampler(const PhysicalDevice &physicalDevice,
                                     const Device &device) {
  VkPhysicalDeviceProperties properties{};
  vkGetPhysicalDeviceProperties(physicalDevice, &properties);

  VkSamplerCreateInfo samplerInfo{};
  samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  samplerInfo.magFilter = VK_FILTER_LINEAR;
  samplerInfo.minFilter = VK_FILTER_LINEAR;
  samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerInfo.anisotropyEnable = VK_TRUE;
  samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
  samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
  samplerInfo.unnormalizedCoordinates = VK_FALSE;
  samplerInfo.compareEnable = VK_FALSE;
  samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
  samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
  samplerInfo.minLod = 0.0f;
  samplerInfo.maxLod = static_cast<float>(mipLevels);
  samplerInfo.mipLodBias = 0.0f;

  CHK(vkCreateSampler(device, &samplerInfo, nullptr, &sampler),
      "failed to create texture sampler");
}
