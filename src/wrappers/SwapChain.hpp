#pragma once
#include "../common.hpp"
#include "Device.hpp"
#include "GPU.hpp"
#include "Image.hpp"
#include "Surface.hpp"
#include <optional>
#include <vector>

namespace Vulking {
class SwapChain {
public:
  SwapChain(GPU gpu, Surface surface);
  ~SwapChain();

  operator VkSwapchainKHR() const;

  const std::vector<VkImage> &getImages() const;
  VkFormat getImageFormat() const;
  VkExtent2D getExtent() const;

private:
  Device device;
  VkSwapchainKHR swapChain{};
  Surface surface;
  VkFormat swapChainImageFormat;
  VkExtent2D swapChainExtent;
  std::vector<Image> swapChainImages;

  void createSwapChain(GPU gpu, Surface surface);
};
} // namespace Vulking
