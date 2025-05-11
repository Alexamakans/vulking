#include "GPU.hpp"

#include "../helpers/VulkingUtil.hpp"
#include <vulkan/vulkan_core.h>

Vulking::GPU::GPU(const Instance &instance, const Surface &surface)
    : instance(instance), surface(surface), physicalDevice(instance, surface),
      device(physicalDevice), graphicsQueue(device, getGraphicsQueueFamily()),
      presentQueue(device, getPresentQueueFamily()),
      renderPass(device, physicalDevice.getFormat(),
                 VulkingUtil::findDepthFormat(physicalDevice),
                 physicalDevice.getMsaaSamples()),
      commandPool(physicalDevice, device) {
  NAME_OBJECT(device, VK_OBJECT_TYPE_QUEUE, (VkQueue)graphicsQueue,
              "graphics_queue");
  NAME_OBJECT(device, VK_OBJECT_TYPE_QUEUE, (VkQueue)presentQueue,
              "present_queue");
}

uint32_t Vulking::GPU::getGraphicsQueueFamily() const {
  assert(physicalDevice.queueFamilyIndices.isComplete());
  return physicalDevice.queueFamilyIndices.graphicsFamily.value();
}

uint32_t Vulking::GPU::getPresentQueueFamily() const {
  assert(physicalDevice.queueFamilyIndices.isComplete());
  return physicalDevice.queueFamilyIndices.presentFamily.value();
}

const Vulking::RenderPass &Vulking::GPU::getRenderPass() const {
  return renderPass;
}

const Vulking::CommandPool &Vulking::GPU::getCommandPool() const {
  return commandPool;
}

const Vulking::DescriptorPool &Vulking::GPU::getDescriptorPool() const {
  return *descriptorPool;
}
