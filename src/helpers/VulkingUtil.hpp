#pragma once

#include "../common.hpp"
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_core.h>

namespace Vulking {
class PhysicalDevice;
class Device;
class CommandPool;
class Queue;
struct Vertex;
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

void createBuffer(const Vulking::PhysicalDevice &physicalDevice,
                  const Vulking::Device &device, VkDeviceSize size,
                  VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
                  VkBuffer &buffer, VkDeviceMemory &memory);

void copyBuffer(const Vulking::Device &device,
                const Vulking::CommandPool &commandPool,
                const Vulking::Queue &queue, VkBuffer src, VkBuffer dst,
                VkDeviceSize size);

void copyBufferToImage(const Vulking::Device &device,
                       const Vulking::CommandPool &commandPool,
                       const Vulking::Queue &queue, VkBuffer buffer,
                       VkImage image, uint32_t width, uint32_t height);

VkCommandBuffer beginCommand(const Vulking::Device &device,
                             const Vulking::CommandPool &commandPool);

void endCommand(const Vulking::Device &device,
                const Vulking::CommandPool &commandPool,
                const Vulking::Queue &queue, VkCommandBuffer commandBuffer);

void transitionImageLayout(const Vulking::Device &device,
                           const Vulking::CommandPool &commandPool,
                           const Vulking::Queue &queue, VkImage image,
                           VkFormat format, VkImageLayout oldLayout,
                           VkImageLayout newLayout, uint32_t mipLevels);

void generateMipmaps(const Vulking::PhysicalDevice &physicalDevice,
                     const Vulking::Device &device,
                     const Vulking::CommandPool &commandPool,
                     const Vulking::Queue &queue, VkImage image,
                     VkFormat imageFormat, int32_t texWidth, int32_t texHeight,
                     uint32_t mipLevels);

void loadModel(const std::string &path, std::vector<Vulking::Vertex> &vertices,
               std::vector<uint32_t> &indices);

} // namespace VulkingUtil
