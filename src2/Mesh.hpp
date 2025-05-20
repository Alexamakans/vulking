#pragma once

#include "Buffer.hpp"
#include "Common.hpp"

namespace Vulking {
class Mesh {
public:
  using Index = uint32_t;
  vk::IndexType IndexType = vk::IndexType::eUint32;

  struct Vertex {
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec2 texCoord;

    static vk::VertexInputBindingDescription getBindingDescription() {
      vk::VertexInputBindingDescription bindingDescription{};
      bindingDescription.binding = 0;
      bindingDescription.stride = sizeof(Vertex);
      bindingDescription.inputRate = vk::VertexInputRate::eVertex;

      return bindingDescription;
    }

    static std::vector<vk::VertexInputAttributeDescription>
    getAttributeDescriptions() {
      std::vector<vk::VertexInputAttributeDescription> attributeDescriptions(3);

      attributeDescriptions[0] = vk::VertexInputAttributeDescription(
          0, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, pos));
      attributeDescriptions[1] = vk::VertexInputAttributeDescription(
          0, 1, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, color));
      attributeDescriptions[2] = vk::VertexInputAttributeDescription(
          0, 2, vk::Format::eR32G32Sfloat, offsetof(Vertex, texCoord));

      return attributeDescriptions;
    }

    bool operator==(const Vertex &other) const;
  };

public:
  Mesh();
  Mesh(const std::string &path, const char *name = "unnamed");
  Mesh(const std::vector<Vertex> &vertices, const std::vector<Index> &indices,
       const char *name = "unnamed");

  void releaseCPUResources();
  void bind(vk::CommandBuffer cmd);

private:
  void init(const char *name = "unnamed");

  std::vector<Vertex> cpuVertices;
  std::vector<Index> cpuIndices;

  Buffer<Vertex> vertices;
  Buffer<Index> indices;
};
} // namespace Vulking

namespace std {
template <> struct hash<Vulking::Mesh::Vertex> {
  size_t operator()(Vulking::Mesh::Vertex const &vertex) const {
    return ((hash<glm::vec3>()(vertex.pos) ^
             (hash<glm::vec3>()(vertex.color) << 1)) >>
            1) ^
           (hash<glm::vec2>()(vertex.texCoord) << 1);
  }
};
} // namespace std
