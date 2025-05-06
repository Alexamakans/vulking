#include "swapchain_image.hpp"
#include "../common.hpp"
#include "../engine.hpp"
#include <vulkan/vulkan_core.h>

SwapchainImage::SwapchainImage(Swapchain swapchain, VkImage image) {
};
SwapchainImage::~SwapchainImage() {
  std::destroy_at(view);
}
