#pragma once
#include "common.hpp"
#include <vector>

class Instance {
public:
  explicit Instance(bool enableValidation = enableValidationLayers);
  ~Instance();

  operator VkInstance() const;

private:
  VkInstance instance{};
  bool validationEnabled;

  void createInstance();
  std::vector<const char *> getRequiredExtensions() const;
  bool checkValidationLayerSupport() const;

  const std::vector<const char *> validationLayers = {
      "VK_LAYER_KHRONOS_validation"};
};
