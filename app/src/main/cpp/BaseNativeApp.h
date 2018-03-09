#ifndef VULLKANAPP_H
#define VULLKANAPP_H

#include <android_native_app_glue.h>
#include "AndroidLogging.h"

template <class UserData>
class BaseNativeApp {
	public:
		~BaseNativeApp();
		void run(android_app* app);
		void handleAppCommand(struct android_app* app, int32_t command);
		int32_t handleInput(struct android_app* app, AInputEvent* event);

	protected:
		virtual UserData* createUserData() = 0;
		UserData* geUserData();

	private:
		UserData* userData;
		android_app* application;
		static void delegateAppCommand(struct android_app* app, int32_t command);
		static int32_t delegateInputEvent(struct android_app* app, AInputEvent* event);
		void processEvents(android_app *app);
};

template <class UserData>
BaseNativeApp<UserData>::~BaseNativeApp() {
	if(userData != nullptr) {
		delete userData;
	}
}

template <class UserData>
void BaseNativeApp<UserData>::run(android_app* app) {
	application = app;
	userData = createUserData();

	app -> userData = this;
	app -> onAppCmd = delegateAppCommand;
	app -> onInputEvent = delegateInputEvent;

	processEvents(app);
}

template <class UserData>
void BaseNativeApp<UserData>::delegateAppCommand(struct android_app* app, int32_t command) {
	BaseNativeApp * android = reinterpret_cast<BaseNativeApp *>(app->userData);
	android -> handleAppCommand(app, command);
}

template <class UserData>
int32_t BaseNativeApp<UserData>::delegateInputEvent(struct android_app* app, AInputEvent* event) {
	BaseNativeApp * android = reinterpret_cast<BaseNativeApp *>(app->userData);
	return android -> handleInput(app, event);
}

template <class UserData>
void BaseNativeApp<UserData>::processEvents(android_app *app) {
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

template <class UserData>
void BaseNativeApp<UserData>::handleAppCommand(struct android_app* app, int32_t command) {
	LOG_INFO("App event observed: %d", command);

	switch(command) {
		case APP_CMD_INIT_WINDOW:
//				initializeDisplay(app);
			break;
		case APP_CMD_TERM_WINDOW:
//				deinitializeDisplay(app);
			break;
	}
}

template <class UserData>
int32_t BaseNativeApp<UserData>::handleInput(struct android_app* app, AInputEvent* event) {
	if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_KEY) {
		LOG_INFO("Input observed: %d", AKeyEvent_getKeyCode(event));

		return 1;
	}

	return 0;
}

template <class UserData>
UserData* BaseNativeApp<UserData>::geUserData() {
	return userData;
}

#endif
