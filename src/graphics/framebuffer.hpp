#pragma once

#include "image.hpp"
#include <vulkan/vulkan_core.h>

class Framebuffer {
public:
  Framebuffer(VkFormat format, int width, int height);
  ~Framebuffer();

  operator Image() const { return image; }
  operator VkFramebuffer() const { return framebuffer; }

private:
  Image image;
  VkFramebuffer framebuffer;
};
