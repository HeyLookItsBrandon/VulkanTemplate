#include <android_native_app_glue.h>
#include "AndroidLogging.h"

#include "BaseNativeApp.h"

void BaseNativeApp::run(android_app* app) {
	application = app;

	app -> userData = this;
	app -> onAppCmd = delegateAppCommand;
	app -> onInputEvent = delegateInputEvent;

	processEvents(app);
}

void BaseNativeApp::delegateAppCommand(struct android_app* app, int32_t command) {
	BaseNativeApp* android = reinterpret_cast<BaseNativeApp*>(app->userData);
	android -> handleAppCommand(app, command);
}

int32_t BaseNativeApp::delegateInputEvent(struct android_app* app, AInputEvent* event) {
	BaseNativeApp* android = reinterpret_cast<BaseNativeApp*>(app->userData);
	return android -> handleInput(app, event);
}

void BaseNativeApp::processEvents(android_app *app) {
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

void BaseNativeApp::handleAppCommand(struct android_app* app, int32_t command) {
	switch(command) {
		case APP_CMD_INIT_WINDOW:
			onWindowInitialized();
			break;
		case APP_CMD_TERM_WINDOW:
			onWindowTerminated();
			break;
		case APP_CMD_START:
			onStart();
			break;
		case APP_CMD_RESUME:
			onResume();
			break;
		case APP_CMD_PAUSE:
			onPause();
			break;
		case APP_CMD_STOP:
			onStop();
			break;
		case APP_CMD_DESTROY:
			onDestroy();
			break;

		case APP_CMD_INPUT_CHANGED:
			onInputChanged();
			break;
		case APP_CMD_WINDOW_RESIZED:
			onWindowResized();
			break;
		case APP_CMD_WINDOW_REDRAW_NEEDED:
			onWindowRedrawNeeded();
			break;
		case APP_CMD_CONTENT_RECT_CHANGED:
			onContentRectChanged();
			break;
		case APP_CMD_GAINED_FOCUS:
			onFocusGained();
			break;
		case APP_CMD_LOST_FOCUS:
			onFocusLost();
			break;
		case APP_CMD_CONFIG_CHANGED:
			onConfigChanged();
			break;
		case APP_CMD_LOW_MEMORY:
			onLowMemory();
			break;
		case APP_CMD_SAVE_STATE:
			onSaveInstanceState();
			break;
		default:
			LOG_ERROR("Unknown application event encountered: %d", command);
			break;
	}
}

android_app* BaseNativeApp::getApplication() {
	return application;
}

int32_t BaseNativeApp::handleInput(struct android_app* app, AInputEvent* event) {
	if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_KEY) {
		LOG_INFO("Input observed: %d", AKeyEvent_getKeyCode(event));

		return 1;
	}

	return 0;
}

void BaseNativeApp::onWindowInitialized() {}
void BaseNativeApp::onWindowTerminated() {}
void BaseNativeApp::onStart() {}
void BaseNativeApp::onResume() {}
void BaseNativeApp::onPause() {}
void BaseNativeApp::onStop() {}
void BaseNativeApp::onDestroy() {}
void BaseNativeApp::onInputChanged() {}
void BaseNativeApp::onWindowResized() {}
void BaseNativeApp::onWindowRedrawNeeded() {}
void BaseNativeApp::onContentRectChanged() {}
void BaseNativeApp::onFocusGained() {}
void BaseNativeApp::onFocusLost() {}
void BaseNativeApp::onConfigChanged() {}
void BaseNativeApp::onLowMemory() {}
void BaseNativeApp::onSaveInstanceState() {}
