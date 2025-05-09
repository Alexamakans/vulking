#include "ImageView.hpp"
#include <vulkan/vulkan_core.h>

Vulking::ImageView::ImageView(const Device &device, VkImage image,
                              VkFormat format, VkImageAspectFlagBits flags,
                              uint32_t mipLevels)
    : device(device), view(createImageView(image, format, flags, mipLevels)) {}

void Vulking::ImageView::release() {
  vkDestroyImageView(device, view, allocator);
}

Vulking::ImageView::operator VkImageView() const { return view; }
VkImageView
Vulking::ImageView::createImageView(VkImage image, VkFormat format,
                                    VkImageAspectFlagBits aspectFlags,
                                    uint32_t mipLevels) {
  VkImageView view;
  VkImageViewCreateInfo info{};
  info.sType = STYPE(IMAGE_VIEW_CREATE_INFO);
  info.image = image;
  info.viewType = VK_IMAGE_VIEW_TYPE_2D;
  info.format = format;
  info.subresourceRange.aspectMask = aspectFlags;
  info.subresourceRange.baseMipLevel = 0;
  info.subresourceRange.levelCount = mipLevels;
  info.subresourceRange.baseArrayLayer = 0;
  info.subresourceRange.layerCount = 1;

  CHK(vkCreateImageView(device, &info, allocator, &view),
      "failed to create image view");

  return view;
}
