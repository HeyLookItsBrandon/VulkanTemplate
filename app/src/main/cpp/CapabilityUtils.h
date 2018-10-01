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
	if(layerCount > 0) {
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
	}

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
	uint32_t count = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);

	std::vector<VkExtensionProperties> extensions(count);
	if(count > 0) {
		vkEnumerateInstanceExtensionProperties(nullptr, &count, extensions.data());
	}

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
	uint32_t count = 0;
	vkEnumeratePhysicalDevices(instance, &count, nullptr);

	std::vector<VkPhysicalDevice> devices(count);
	if(count > 0) {
		vkEnumeratePhysicalDevices(instance, &count, devices.data());
	}

	return devices;
}

std::vector<VkQueueFamilyProperties> getQueueFamilyProperties(VkPhysicalDevice device) {
	uint32_t count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &count, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(count);
	if(count > 0) {
		vkGetPhysicalDeviceQueueFamilyProperties(device, &count, queueFamilies.data());
	}

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
	uint32_t count;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &count, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(count);
	if(count > 0) {
		vkEnumerateDeviceExtensionProperties(device, nullptr, &count, availableExtensions.data());
	}

	return availableExtensions;
}

void logPhysicalDeviceExtensionProperties(const VkPhysicalDevice& device) {
	std::vector<VkExtensionProperties> properties = getPhysicalDeviceExtensionProperties(device);
	LOG_DEBUG("Found %lu supported physical device extensions.", properties.size());
	for(const VkExtensionProperties& property : properties) {
		LOG_DEBUG("%s (%i)",
				property.extensionName,
				property.specVersion);
	}
}

std::vector<VkSurfaceFormatKHR> getPhysicalDeviceSurfaceFormats(
		const VkPhysicalDevice& device, const VkSurfaceKHR& surface) {
	uint32_t count;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &count, nullptr);

	std::vector<VkSurfaceFormatKHR> formats(count);
	if(count > 0) {
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &count, formats.data());
	}

	return formats;
}

std::vector<VkPresentModeKHR> getPhysicalDeviceSurfacePresentModes(
		const VkPhysicalDevice& device, const VkSurfaceKHR& surface) {
	uint32_t count;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &count, nullptr);

	std::vector<VkPresentModeKHR> modes(count);
	if (count > 0) {
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &count, modes.data());
	}

	return modes;
}

VkSurfaceCapabilitiesKHR getPhysicalDeviceSurfaceCapabilities(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) {
	VkSurfaceCapabilitiesKHR capabilities = {};
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &capabilities);

	return capabilities;
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

VkBool32 isPresentationSupported(const VkPhysicalDevice& physicalDevice, unsigned int queueFamilyIndex, const VkSurfaceKHR& surface) {
	VkBool32 presentSupport = VK_FALSE;
	vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, queueFamilyIndex, surface, &presentSupport);

	return presentSupport;
}

void getSwapchainImages(VkDevice  device, VkSwapchainKHR swapchain, std::vector<VkImage>& images) {
	uint32_t count = 0;
	vkGetSwapchainImagesKHR(device, swapchain, &count, nullptr);
	images.resize(count);
	vkGetSwapchainImagesKHR(device, swapchain, &count, images.data());
}

VkShaderModule createShaderModule(const VkDevice& device, const std::vector<char>& shaderBytecode) {
	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = shaderBytecode.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(shaderBytecode.data());

	VkShaderModule shaderModule;
	assertSuccess(vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule), "Failed to create shader module.");

	return shaderModule;
}

VkMemoryRequirements getBufferMemoryRequirements(const VkDevice& device, const VkBuffer& buffer) {
	VkMemoryRequirements requirements;
	vkGetBufferMemoryRequirements(device, buffer, &requirements);

	return requirements;
}

VkPhysicalDeviceMemoryProperties getPhysicalDeviceMemoryProperties(const VkPhysicalDevice& physicalDevice) {
	VkPhysicalDeviceMemoryProperties properties;
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &properties);

	return properties;
}

#endif
