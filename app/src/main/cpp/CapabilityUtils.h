#ifndef CAPABILITYQUERIER_H
#define CAPABILITYQUERIER_H

#include <string>
#include <cstring>
#include <vector>
#include "vulkan_wrapper/vulkan_wrapper.h"
#include "AndroidLogging.h"
#include "CollectionUtils.h"


void assertSuccess(VkResult result, std::string message) {
	if(result != VK_SUCCESS) {
		throw std::runtime_error(message.c_str());
	}
}

VkResult createDebugReportCallback(VkInstance instance,
		const VkDebugReportCallbackCreateInfoEXT* pCreateInfo,
		const VkAllocationCallbacks* pAllocator,
		VkDebugReportCallbackEXT* pCallback) {
	if(vkCreateDebugReportCallbackEXT == nullptr) {
		vkCreateDebugReportCallbackEXT = reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(
				vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT"));
	}

	return vkCreateDebugReportCallbackEXT(instance, pCreateInfo, pAllocator, pCallback);
}

void destroyDebugReportCallback(VkInstance instance,
		VkDebugReportCallbackEXT callback,
		const VkAllocationCallbacks* pAllocator) {
	if(vkDestroyDebugReportCallbackEXT == nullptr) {
		vkDestroyDebugReportCallbackEXT = reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>(
				vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT"));
	}

	vkDestroyDebugReportCallbackEXT(instance, callback, pAllocator);
}

std::vector<VkLayerProperties> getSupportedValidationLayers() {
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	return availableLayers;
}

void logSupportedValidationLayers() {
	std::vector<VkLayerProperties> layers = getSupportedValidationLayers();

	LOG_DEBUG("Found %lu supported validation layers.", layers.size());
	for(const VkLayerProperties& layer : layers) {
		LOG_DEBUG("%s (%u) - %s",
				layer.layerName,
				layer.implementationVersion,
				layer.description);
	}
}

std::vector<const char *> filterUnavailableValidationLayers(
		std::vector<const char *> requestedLayerNames) {
	std::vector<VkLayerProperties> supportedLayers = getSupportedValidationLayers();
	std::vector<const char *> supportedLayerNames;

	for(const char* requestedLayerName : requestedLayerNames) {
		for(const VkLayerProperties& supportedLayer : supportedLayers) {
			if(strcmp(requestedLayerName, supportedLayer.layerName) == 0) {
				supportedLayerNames.push_back(requestedLayerName);

				// Workaround for no labeled loops in C++, to continue the outer loop
				goto skipUnsupportedLogging;
			}
		}

		LOG_WARN("Unsupported validation layer requested: %s", requestedLayerName);

		skipUnsupportedLogging:;
	}

	return supportedLayerNames;
}

std::vector<VkExtensionProperties> getSupportedInstanceExtensions() {
	uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> extensions(extensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

	return extensions;
}

void logSupportedInstanceExtensions() {
	std::vector<VkExtensionProperties> extensions = getSupportedInstanceExtensions();

	LOG_DEBUG("Found %lu supported instance extensions.", extensions.size());
	for(const VkExtensionProperties& extension : extensions) {
		LOG_DEBUG("%s (%i)",
				extension.extensionName,
				extension.specVersion);
	}
}

std::vector<VkPhysicalDevice> getPhysicalDevices(VkInstance instance) {
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

	return devices;
}

std::vector<VkQueueFamilyProperties> getQueueFamilyProperties(VkPhysicalDevice device) {
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	return queueFamilies;
}

VkPhysicalDeviceProperties getPhysicalDeviceProperties(VkPhysicalDevice device) {
	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);

	return deviceProperties;
}

VkPhysicalDeviceFeatures getPhysicalDeviceFeatures(VkPhysicalDevice device) {
	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

	return deviceFeatures;
}

std::vector<VkExtensionProperties> getPhysicalDeviceExtensionProperties(VkPhysicalDevice device) {
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

	return availableExtensions;
}

bool arePhysicalDeviceExtensionSupported(VkPhysicalDevice device,
		std::vector<const char*> requiredExtensionNames) {
	std::vector<VkExtensionProperties> supportedDeviceExtensions =
			getPhysicalDeviceExtensionProperties(device);

	// Note that this coerces the C strings to STL strings because our use of a map relies on string's equality implementation
	std::set<std::string> remainingRequiredExtensionNames = asSet<const char*, std::string>(requiredExtensionNames);
	for (const VkExtensionProperties& deviceExtension : supportedDeviceExtensions) {
		remainingRequiredExtensionNames.erase(deviceExtension.extensionName);
		if(remainingRequiredExtensionNames.empty()) {
			return true;
		}
	}

	return false;
}

VkBool32 isPresentationSupported(const VkPhysicalDevice& physicalDevice, int queueFamilyIndex, const VkSurfaceKHR& surface) {
	VkBool32 presentSupport = VK_FALSE;
	vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, queueFamilyIndex, surface, &presentSupport);

	return presentSupport;
}

#endif
