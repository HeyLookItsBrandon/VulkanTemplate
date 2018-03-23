#include <string.h>
#include "CapabilityUtil.h"
#include "AndroidLogging.h"

std::vector<VkLayerProperties> getSupportedValidationLayers() {
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	return availableLayers;
}

void logSupportedValidationLayers() {
	std::vector<VkLayerProperties> layers = getSupportedValidationLayers();

	LOG_INFO("Found %lu supported validation layers.", layers.size());
	for(int i=0; i<layers.size(); i++) {
		VkLayerProperties layer = layers[i];

		LOG_INFO("%s (%u) - %s",
				 layer.layerName,
				 layer.implementationVersion,
				 layer.description);
	}
}

std::vector<const char *> filterUnavailableValidationLayers(
		std::vector<const char *> requestedLayerNames) {
	std::vector<VkLayerProperties> supportedLayers = getSupportedValidationLayers();
	std::vector<const char *> supportedLayerNames;

	for(int i = 0; i < requestedLayerNames.size(); i++) {
		const char* requestedLayerName = requestedLayerNames[i];

		for(int j = 0; j < supportedLayers.size(); j++) {
			VkLayerProperties supportedLayer = supportedLayers[j];

			if(strcmp(requestedLayerName, supportedLayer.layerName) == 0) {
				supportedLayerNames.push_back(requestedLayerName);

				// Workaround for C++ not having labeled loop/continues, to continue the outer loop
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

	LOG_INFO("Found %lu supported instance extensions.", extensions.size());
	for(int i=0; i < extensions.size(); i++) {
		VkExtensionProperties extension = extensions[i];

		LOG_INFO("%s (%i)",
				 extension.extensionName,
				 extension.specVersion);
	}
}
