#include "Vertex.hpp"

bool Vulking::Vertex::operator==(const Vertex &other) const {
  return pos == other.pos && color == other.color && texCoord == other.texCoord;
}
