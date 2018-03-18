#include "VulkanNativeApp.h"
#include "AndroidLogging.h"
#include "CapabilityUtil.h"

const std::vector<const char *> INSTANCE_EXTENSIONS_NAMES = {
		VK_KHR_SURFACE_EXTENSION_NAME,
		VK_KHR_ANDROID_SURFACE_EXTENSION_NAME };

const std::vector<const char *> DEVICE_EXTENSIONS_NAMES = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME };

const std::vector<const char *> VALIDATION_LAYER_NAMES = {
		"VK_LAYER_LUNARG_core_validation",
		"VK_LAYER_LUNARG_parameter_validation" };


VulkanNativeApp::VulkanNativeApp() {
	InitVulkan();
}

void VulkanNativeApp::onWindowInitialized() {
	vulkanInstance = initializeDisplay();
}

void VulkanNativeApp::onWindowTerminated() {
	deinitializeDisplay(vulkanInstance);
}

VkInstance* VulkanNativeApp::initializeDisplay() {
	logSupportedInstanceExtensions();
	logSupportedValidationLayers();

	VkApplicationInfo appInfo = createApplicationInfo();
	VkInstanceCreateInfo instanceCreationInfo = createInstanceCreationInfo(appInfo);

	VkInstance* instance = new VkInstance;
	VkResult result = vkCreateInstance(&instanceCreationInfo, nullptr, instance);
	LOG_INFO("Instance creation result: %d", result);

	return instance;
}

void VulkanNativeApp::deinitializeDisplay(VkInstance* instance) {
	vkDestroyInstance(*instance, nullptr);
}

VkApplicationInfo VulkanNativeApp::createApplicationInfo() {
	VkApplicationInfo info = {};

	info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	info.pApplicationName = "Vulkan Template";
	info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	info.pEngineName = "No Engine";
	info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	info.apiVersion = VK_API_VERSION_1_0;

	return info;
}

VkInstanceCreateInfo VulkanNativeApp::createInstanceCreationInfo(VkApplicationInfo& applicationInfo) {
	VkInstanceCreateInfo createInfo = {};

	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &applicationInfo;

	createInfo.enabledExtensionCount = (uint32_t) INSTANCE_EXTENSIONS_NAMES.size();
	createInfo.ppEnabledExtensionNames = INSTANCE_EXTENSIONS_NAMES.data();

	std::vector<const char *> availableLayers = pruneValidationLayers(VALIDATION_LAYER_NAMES);
	createInfo.enabledLayerCount = (uint32_t) availableLayers.size();
	createInfo.ppEnabledLayerNames = availableLayers.data();

	return createInfo;
}
