#pragma once
#include "../common.hpp"
#include <vector>


namespace Vulking {
class Instance {
public:
  explicit Instance(bool enableValidation = enableValidationLayers);
  Instance(Instance &) = default;
  ~Instance();

  operator VkInstance() const;

private:
  VkInstance instance{};
  bool validationEnabled;

  void createInstance();
  std::vector<const char *> getRequiredExtensions() const;
  bool checkValidationLayerSupport() const;
};
} // namespace Vulking
