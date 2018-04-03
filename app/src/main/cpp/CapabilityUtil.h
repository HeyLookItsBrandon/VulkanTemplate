#ifndef CAPABILITYQUERIER_H
#define CAPABILITYQUERIER_H

#include <string>
#include <vector>
#include "vulkan_wrapper/vulkan_wrapper.h"

std::vector<VkLayerProperties> getSupportedValidationLayers();
void logSupportedValidationLayers();
std::vector<const char *> filterUnavailableValidationLayers(
		std::vector<const char *> requestedLayerNames);

std::vector<VkExtensionProperties> getSupportedInstanceExtensions();
void logSupportedInstanceExtensions();

std::vector<VkPhysicalDevice> getPhysicalDevices(VkInstance instance);

std::vector<VkQueueFamilyProperties> getQueueFamilyProperties(VkPhysicalDevice device);

void assertSuccess(VkResult result, std::string message);

VkPhysicalDeviceProperties getPhysicalDeviceProperties(VkPhysicalDevice device);
VkPhysicalDeviceFeatures getPhysicalDeviceFeatures(VkPhysicalDevice device);

#endif
