#pragma once
#include "Device.hpp"
#include "Image.hpp"
#include "Queue.hpp"
#include "Surface.hpp"
#include "common.hpp"
#include <optional>
#include <vector>

class SwapChain {
public:
  SwapChain(const Device &deviceInfo);
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

  void createSwapChain(const Device &deviceInfo, VkSurfaceKHR surface);
};
