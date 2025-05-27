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
      auto bindingDescription = vk::VertexInputBindingDescription{}
                                    .setBinding(0)
                                    .setStride(sizeof(Vertex))
                                    .setInputRate(vk::VertexInputRate::eVertex);

      return bindingDescription;
    }

    static std::vector<vk::VertexInputAttributeDescription>
    getAttributeDescriptions() {
      auto vec3 = vk::Format::eR32G32B32Sfloat;
      auto vec2 = vk::Format::eR32G32Sfloat;
      std::vector<vk::VertexInputAttributeDescription> attributeDescriptions(3);

      attributeDescriptions[0] = vk::VertexInputAttributeDescription()
                                     .setBinding(0)
                                     .setLocation(0)
                                     .setFormat(vec3)
                                     .setOffset(offsetof(Vertex, pos));
      attributeDescriptions[1] = vk::VertexInputAttributeDescription()
                                     .setBinding(0)
                                     .setLocation(2)
                                     .setFormat(vec3)
                                     .setOffset(offsetof(Vertex, color));
      attributeDescriptions[2] = vk::VertexInputAttributeDescription()
                                     .setBinding(0)
                                     .setLocation(3)
                                     .setFormat(vec2)
                                     .setOffset(offsetof(Vertex, texCoord));

      return attributeDescriptions;
    }

    bool operator==(const Vertex &other) const {
      return pos == other.pos && color == other.color &&
             texCoord == other.texCoord;
    }
  };

public:
  Mesh();
  Mesh(const std::string &path, const char *name = "unnamed");
  Mesh(const std::vector<Vertex> &vertices, const std::vector<Index> &indices,
       const char *name = "unnamed");

  void releaseCPUResources();
  void bind(vk::CommandBuffer cmd);

  uint32_t getNumVertices() const { return numVertices; }
  uint32_t getNumIndices() const { return numIndices; }

private:
  void init(const char *name = "unnamed");

  std::vector<Vertex> cpuVertices;
  uint32_t numVertices;
  std::vector<Index> cpuIndices;
  uint32_t numIndices;

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
