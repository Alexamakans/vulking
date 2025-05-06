#pragma once

#include <vulkan/vulkan_core.h>

class ImageView {
public:
  ImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags,
            uint32_t mipLevels);
  ~ImageView();

  operator VkImageView() const { return view; }
  operator VkImageView *() { return &view; }

private:
  VkImageView view;
};
