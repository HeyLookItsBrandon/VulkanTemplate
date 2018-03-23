#ifndef CAPABILITYQUERIER_H
#define CAPABILITYQUERIER_H

#include <vector>
#include "vulkan_wrapper/vulkan_wrapper.h"

std::vector<VkLayerProperties> getSupportedValidationLayers();
void logSupportedValidationLayers();
std::vector<const char *> filterUnavailableValidationLayers(
		std::vector<const char *> requestedLayerNames);

std::vector<VkExtensionProperties> getSupportedInstanceExtensions();
void logSupportedInstanceExtensions();

#endif
