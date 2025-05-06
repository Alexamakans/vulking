#include "image_view.hpp"
#include "../common.hpp"
#include "../engine.hpp"

ImageView::ImageView(VkImage image, VkFormat format,
                     VkImageAspectFlags aspectFlags, uint32_t mipLevels) {
  VkImageViewCreateInfo createInfo{};
  createInfo.sType = STYPE(IMAGE_VIEW_CREATE_INFO);

  createInfo.image = image;
  createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
  createInfo.format = format;

  createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
  createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
  createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
  createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

  createInfo.subresourceRange.aspectMask = aspectFlags;
  createInfo.subresourceRange.baseMipLevel = 0;
  createInfo.subresourceRange.levelCount = mipLevels;
  createInfo.subresourceRange.baseArrayLayer = 0;
  createInfo.subresourceRange.layerCount = 1;

  CHK(vkCreateImageView(Engine::device, &createInfo, nullptr, &view),
      "failed to create texture image view");
}

ImageView::~ImageView() { vkDestroyImageView(Engine::device, view, allocator); }
