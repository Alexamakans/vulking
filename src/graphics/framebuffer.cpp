#include "framebuffer.hpp"
#include "../engine.hpp"
#include "image.hpp"
#include <vulkan/vulkan_core.h>

VkFormat findDepthFormat() {
  return Engine::gpu.findSupportedFormat(
      {
          VK_FORMAT_D32_SFLOAT,
          VK_FORMAT_D32_SFLOAT_S8_UINT,
          VK_FORMAT_D24_UNORM_S8_UINT,
      },
      VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

Framebuffer::Framebuffer(int width, int height, VkFormat format,
                         SwapchainImage swapchainImage)
    : color(Image(width, height, format,
                  VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT |
                      VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                  1, Engine::gpu.msaaSamples, VK_IMAGE_ASPECT_COLOR_BIT)),
      depth(Image(width, height, findDepthFormat(),
                  VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, 1,
                  Engine::gpu.msaaSamples, VK_IMAGE_ASPECT_DEPTH_BIT)),
      swapchainImage(swapchainImage) {
  std::array<VkImageView, 3> attachments = {
      color,
      depth,
      image,
  };
}
