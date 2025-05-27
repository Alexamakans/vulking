#include "Mesh.hpp"
#include "Buffer.hpp"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

namespace Vulking {
static void loadModel(const std::string &path,
                      std::vector<Mesh::Vertex> &vertices,
                      std::vector<Mesh::Index> &indices);

Mesh::Mesh() {}

Mesh::Mesh(const std::string &path, const char *name) {
  loadModel(path, cpuVertices, cpuIndices);
}

Mesh::Mesh(const std::vector<Vertex> &vertices,
           const std::vector<Index> &indices, const char *name) {
  cpuVertices = vertices;
  cpuIndices = indices;
}

void Mesh::releaseCPUResources() {
  cpuVertices.clear();
  cpuVertices.shrink_to_fit();
  cpuIndices.clear();
  cpuIndices.shrink_to_fit();
}

void Mesh::bind(vk::CommandBuffer cmd) {
  vk::Buffer vertexBuffers[] = {vertices.getBuffer()};
  vk::DeviceSize offsets[] = {0};
  cmd.bindVertexBuffers(0, 1, vertexBuffers, offsets);
  cmd.bindIndexBuffer(indices.getBuffer(), 0, IndexType);
}

void Mesh::init(const char *name) {
  numVertices = static_cast<uint32_t>(cpuVertices.size());
  numIndices = static_cast<uint32_t>(cpuIndices.size());
  auto verticesStaging =
      Buffer<Vertex>(cpuVertices, BufferUsage::STAGING, BufferMemory::STAGING,
                     std::format("{}_vertex_staging", name).c_str());
  vertices = Buffer<Vertex>(verticesStaging.getSize(), BufferUsage::FINAL,
                            BufferMemory::FINAL,
                            std::format("{}_vertex", name).c_str());

  auto indicesStaging =
      Buffer<Index>(cpuIndices, BufferUsage::STAGING, BufferMemory::STAGING,
                    std::format("{}_index_staging", name).c_str());
  indices =
      Buffer<Index>(indicesStaging.getSize(), BufferUsage::FINAL,
                    BufferMemory::FINAL, std::format("{}_index", name).c_str());
}

static void loadModel(const std::string &path,
                      std::vector<Mesh::Vertex> &vertices,
                      std::vector<Mesh::Index> &indices) {
  vertices.clear();
  indices.clear();
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;
  std::string warn, err;
  if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
                        path.c_str())) {
    throw std::runtime_error(warn + err);
  }

  std::unordered_map<Mesh::Vertex, Mesh::Index> uniqueVertices{};

  for (const auto &shape : shapes) {
    for (const auto &index : shape.mesh.indices) {
      Mesh::Vertex vertex{};

      vertex.pos = {
          attrib.vertices[3 * index.vertex_index + 0],
          attrib.vertices[3 * index.vertex_index + 1],
          attrib.vertices[3 * index.vertex_index + 2],
      };

      vertex.texCoord = {
          attrib.texcoords[2 * index.texcoord_index + 0],
          1.0f - attrib.texcoords[2 * index.texcoord_index + 1],
      };

      vertex.color = {1.0f, 1.0f, 1.0f};

      if (uniqueVertices.count(vertex) == 0) {
        uniqueVertices[vertex] = static_cast<Mesh::Index>(vertices.size());
        vertices.push_back(vertex);
      }

      indices.push_back(uniqueVertices[vertex]);
    }
  }
}
} // namespace Vulking
