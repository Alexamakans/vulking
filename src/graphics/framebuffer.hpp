#pragma once

#include "image.hpp"
#include <vulkan/vulkan_core.h>

class Framebuffer {
public:
  Framebuffer() {}
  Framebuffer(int width, int height, VkFormat format);
  ~Framebuffer();

  operator Image() const { return image; }
  operator Image *() { return &image; }
  operator VkFramebuffer() const { return framebuffer; }
  operator VkFramebuffer *() { return &framebuffer; }

private:
  Image image;
  Image color;
  Image depth;
  VkFramebuffer framebuffer;
};
