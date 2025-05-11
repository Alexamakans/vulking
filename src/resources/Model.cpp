#include "Model.hpp"

#include "../helpers/VulkingUtil.hpp"
#include "Vertex.hpp"

Vulking::Model::Model(const PhysicalDevice &physicalDevice,
                      const Device &device, const CommandPool &commandPool,
                      const Queue &queue, const std::string &path)
    : physicalDevice(physicalDevice), device(device) {
  std::vector<Vertex> vertices;
  std::vector<uint32_t> indices;

  VulkingUtil::loadModel(path, vertices, indices);
  numVertices = static_cast<uint32_t>(vertices.size());
  numIndices = static_cast<uint32_t>(indices.size());
  createBuffer(physicalDevice, device, commandPool, queue, vertices,
               VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                   VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
               vertexBuffer, vertexBufferMemory, "model_vertex_buffer");
  createBuffer(physicalDevice, device, commandPool, queue, indices,
               VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                   VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
               indexBuffer, indexBufferMemory, "model_index_buffer");
}

void Vulking::Model::release() const {
  vkDestroyBuffer(device, vertexBuffer, allocator);
  vkFreeMemory(device, vertexBufferMemory, allocator);
  vkDestroyBuffer(device, indexBuffer, allocator);
  vkFreeMemory(device, indexBufferMemory, allocator);
}

void Vulking::Model::bindBuffers(VkCommandBuffer commandBuffer) {
  VkBuffer vertexBuffers[] = {vertexBuffer};
  VkDeviceSize offsets[] = {0};
  vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
  vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);
};

uint32_t Vulking::Model::getNumIndices() const { return numIndices; }
