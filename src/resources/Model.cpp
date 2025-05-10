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
  createBuffer(physicalDevice, device, commandPool, queue, vertices,
               VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                   VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
               vertexBuffer, vertexBufferMemory);
  createBuffer(physicalDevice, device, commandPool, queue, indices,
               VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                   VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
               indexBuffer, indexBufferMemory);
}

Vulking::Model::~Model() { vkDestroyBuffer(device, vertexBuffer, allocator); }
