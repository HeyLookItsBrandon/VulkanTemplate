#include <android_native_app_glue.h>
#include <cstring>

#include "AndroidLogging.h"
#include "vulkan_wrapper/vulkan_wrapper.h"

struct State {
	android_app *app;
};

void initializeDisplay(android_app *app);
void deinitializeDisplay(android_app *app);

void* buildState(android_app *app) {
	struct State state;
	std::memset(&state, 0, sizeof(state));

	state.app = app;

	return &state;
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
	uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

	LOG_INFO("%d extensions supported", extensionCount);
}

void deinitializeDisplay(android_app *app) {

}

void android_main(android_app *app) {
	InitVulkan();

	app -> onAppCmd = handleAppCommand;
	app -> onInputEvent = handleInput;
	app -> userData = buildState(app);

	// Setup if applicable

	processEvents(app);

	// Cleanup if applicable
}
