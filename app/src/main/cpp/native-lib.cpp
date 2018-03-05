#include <android_native_app_glue.h>
#include <vector>

#include "AndroidLogging.h"
#include "vulkan_wrapper/vulkan_wrapper.h"

std::vector<const char *> instanceExtensions = {
		VK_KHR_SURFACE_EXTENSION_NAME,
		VK_KHR_ANDROID_SURFACE_EXTENSION_NAME };

std::vector<const char *> deviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME };

struct State {
	android_app *app;
};

VkApplicationInfo createApplicationInfo() {
	VkApplicationInfo info = {};

	info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	info.pApplicationName = "Hello Triangle";
	info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	info.pEngineName = "No Engine";
	info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	info.apiVersion = VK_API_VERSION_1_0;

	return info;
}

VkInstanceCreateInfo createInstanceCreationInfo(VkApplicationInfo& applicationInfo) {
	VkInstanceCreateInfo createInfo = {};

	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &applicationInfo;

	createInfo.enabledExtensionCount = static_cast<uint32_t>(instanceExtensions.size());
	createInfo.ppEnabledExtensionNames = instanceExtensions.data();

	createInfo.enabledLayerCount = 0;

	return createInfo;
}

void initializeDisplay(android_app *app);
void deinitializeDisplay(android_app *app);


State buildState(android_app *app) {
	struct State state = {};

	state.app = app;

	return state;
}

State getState(android_app *app) {
	return *(struct State *) app->userData;
}

int32_t handleInput(struct android_app* app, AInputEvent* event) {
	if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_KEY) {
		LOG_INFO("Input observed: %d", AKeyEvent_getKeyCode(event));

		return 1;
	}

	return 0;
}

void processEvents(android_app *app) {
	while(true) {
		struct android_poll_source *source;
		// An interval of -1 will cause ALooper_pollAll to block. Considering using 0 while doing
		// work that should continue regardless of an event happening
		while (ALooper_pollAll(-1, nullptr, nullptr, (void **) &source) >= 0) {
			if (source != NULL) {
				source -> process(app, source);
			}

			if (app->destroyRequested != 0) {
				return;
			}
		}

		// Make updated and re-render
	}
}

void handleAppCommand(struct android_app* app, int32_t command) {
	LOG_INFO("App event observed: %d", command);

	switch(command) {
		case APP_CMD_INIT_WINDOW:
			initializeDisplay(app);
			break;
		case APP_CMD_TERM_WINDOW:
			deinitializeDisplay(app);
			break;
	}
}

void initializeDisplay(android_app *app) {
	VkApplicationInfo appInfo = createApplicationInfo();
	VkInstanceCreateInfo instanceCreationInfo = createInstanceCreationInfo(appInfo);

	VkInstance instance;
	VkResult result = vkCreateInstance(&instanceCreationInfo, nullptr, &instance);

	LOG_INFO("Instance creation result: %d", result);
}

void deinitializeDisplay(android_app *app) {

}

void android_main(android_app *app) {
	InitVulkan();

	app -> onAppCmd = handleAppCommand;
	app -> onInputEvent = handleInput;

	// Declaring this as static solves the problem of leaking a reference to a local variable and
	// since this method reaching its end is essentially the end of the application, this *seems*
	// okay. But I wonder what a better alternatives exist.
	// of going about this.
	static State state = buildState(app);
	app -> userData = &state;

	processEvents(app);
}
