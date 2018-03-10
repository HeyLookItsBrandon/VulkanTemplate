#include "VulkanNativeApp.h"
#include "AndroidLogging.h"

std::vector<const char *> instanceExtensions = {
		VK_KHR_SURFACE_EXTENSION_NAME,
		VK_KHR_ANDROID_SURFACE_EXTENSION_NAME };

std::vector<const char *> deviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME };


void VulkanNativeApp::onWindowInitialized() {
	initializeDisplay(getApplication());
}

void VulkanNativeApp::onWindowTerminated() {
	deinitializeDisplay(getApplication());
}

void VulkanNativeApp::initializeDisplay(android_app *app) {
	VkApplicationInfo appInfo = createApplicationInfo();
	VkInstanceCreateInfo instanceCreationInfo = createInstanceCreationInfo(appInfo);

	VkInstance instance;
	VkResult result = vkCreateInstance(&instanceCreationInfo, nullptr, &instance);

	LOG_INFO("Instance creation result: %d", result);
}

void VulkanNativeApp::deinitializeDisplay(android_app *app) {

}

VkApplicationInfo VulkanNativeApp::createApplicationInfo() {
	VkApplicationInfo info = {};

	info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	info.pApplicationName = "Hello Triangle";
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

	createInfo.enabledExtensionCount = static_cast<uint32_t>(instanceExtensions.size());
	createInfo.ppEnabledExtensionNames = instanceExtensions.data();

	createInfo.enabledLayerCount = 0;

	return createInfo;
}