#pragma once

#include "../common.hpp"
#include "../engine.hpp"

class Texture {
public:
  Texture(VkBuffer buffer, int width, int height) {};
  Texture(VkDevice device) : device(device) {};
  ~Texture() {
    vkDestroyImageView(device, view, allocator);
    vkDestroyImage(device, image, allocator);
    vkFreeMemory(device, memory, allocator);
  };

private:
  VkDevice device;
};
