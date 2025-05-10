#pragma once

#include "../common.hpp"
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_core.h>

namespace Vulking {
class PhysicalDevice;
class Device;
} // namespace Vulking

namespace VulkingUtil {
uint32_t findMemoryType(const Vulking::PhysicalDevice &physicalDevice,
                        uint32_t typeFilter, VkMemoryPropertyFlags properties);

void createImage(const Vulking::PhysicalDevice &physicalDevice,
                 const Vulking::Device &device, uint32_t width, uint32_t height,
                 uint32_t mipLevels, VkSampleCountFlagBits numSamples,
                 VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
                 VkMemoryPropertyFlags properties, VkImage &image,
                 VkDeviceMemory &imageMemory);

VkImageView createImageView(const Vulking::Device &device, VkImage image,
                            VkFormat format, VkImageAspectFlags aspectFlags,
                            uint32_t mipLevels);

VkFormat findSupportedFormat(const Vulking::PhysicalDevice &physicalDevice,
                             const std::vector<VkFormat> &candidates,
                             VkImageTiling tiling,
                             VkFormatFeatureFlags features);

VkFormat findDepthFormat(const Vulking::PhysicalDevice &physicalDevice);
} // namespace VulkingUtil
