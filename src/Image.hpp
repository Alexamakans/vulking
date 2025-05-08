#pragma once

#include "Device.hpp"
#include <vulkan/vulkan.hpp>

class Image {
public:
  Image(const Device &device, uint32_t width, uint32_t height);
  operator VkImage() const;

private:
  const Device &device;
  VkImage image;
};
