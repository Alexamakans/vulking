#pragma once

#include <iostream>
#include <stdexcept>
#include <vulkan/vulkan_core.h>

#define CHK(x, msg)                                                            \
  if (x != VK_SUCCESS) {                                                       \
    throw std::runtime_error(msg);                                             \
  }

#define STYPE(x) VK_STRUCTURE_TYPE_##x

#ifndef VULKING_ALLOCATOR
#define VULKING_ALLOCATOR nullptr
#endif

static const VkAllocationCallbacks *allocator = VULKING_ALLOCATOR;

VkCommandBuffer beginSingleTimeCommands(VkDevice device) {
  VkCommandBufferAllocateInfo allocInfo{
      .sType = STYPE(COMMAND_BUFFER_ALLOCATE_INFO),
      .commandPool = commandPool,
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

void endSingleTimeCommands(VkDevice device, VkCommandBuffer commandBuffer) {
  vkEndCommandBuffer(commandBuffer);

  VkSubmitInfo submitInfo{
      .sType = STYPE(SUBMIT_INFO),
      .commandBufferCount = 1,
      .pCommandBuffers = &commandBuffer,
  };

  CHK(vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE),
      "failed to submit command buffer to queue")
  CHK(vkQueueWaitIdle(graphicsQueue), "failed to wait for queue to be idle")

  vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}
