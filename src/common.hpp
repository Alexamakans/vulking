#pragma once

#include <iostream>
#include <stdexcept>
#include <vulkan/vulkan.hpp>

#define CHK(x, msg)                                                            \
  if (x != VK_SUCCESS) {                                                       \
    throw std::runtime_error(msg);                                             \
  }

#define STYPE(x) VK_STRUCTURE_TYPE_##x

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

#ifndef VULKING_ALLOCATOR
#define VULKING_ALLOCATOR nullptr
#endif

#ifndef ENABLE_SAMPLE_SHADING
#define ENABLE_SAMPLE_SHADING false
#endif

inline static const VkAllocationCallbacks *allocator = VULKING_ALLOCATOR;

inline VkCommandBuffer beginSingleTimeCommands(VkDevice device,
                                               VkCommandPool pool) {
  VkCommandBufferAllocateInfo allocInfo{
      .sType = STYPE(COMMAND_BUFFER_ALLOCATE_INFO),
      .commandPool = pool,
      .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
      .commandBufferCount = 1,
  };

  VkCommandBuffer commandBuffer;
  CHK(vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer),
      "failed to allocate command buffer")

  VkCommandBufferBeginInfo beginInfo{
      .sType = STYPE(COMMAND_BUFFER_BEGIN_INFO),
      .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
  };

  CHK(vkBeginCommandBuffer(commandBuffer, &beginInfo),
      "failed to begin command buffer")

  return commandBuffer;
}

inline void endSingleTimeCommands(VkDevice device, VkCommandPool pool,
                                  VkCommandBuffer commandBuffer,
                                  VkQueue queue) {
  vkEndCommandBuffer(commandBuffer);

  VkSubmitInfo submitInfo{
      .sType = STYPE(SUBMIT_INFO),
      .commandBufferCount = 1,
      .pCommandBuffers = &commandBuffer,
  };

  CHK(vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE),
      "failed to submit command buffer to queue")
  CHK(vkQueueWaitIdle(queue), "failed to wait for queue to be idle")

  vkFreeCommandBuffers(device, pool, 1, &commandBuffer);
}
