#ifndef VULLKANAPP_H
#define VULLKANAPP_H

#include <android_native_app_glue.h>

class BaseNativeApp {
	public:
		BaseNativeApp(android_app* app);

		void run();
		void handleAppCommand(struct android_app* app, int32_t command);
		int32_t handleInput(struct android_app* app, AInputEvent* event);

	protected:
		const android_app* getApplication();
		AAssetManager* getAssetManager();

		virtual void onWindowInitialized();
		virtual void onWindowTerminated();

		virtual void onStart();
		virtual void onResume();
		virtual void onPause();
		virtual void onStop();
		virtual void onDestroy();

		virtual void onInputChanged();
		virtual void onWindowResized();
		virtual void onWindowRedrawNeeded();
		virtual void onContentRectChanged();
		virtual void onFocusGained();
		virtual void onFocusLost();
		virtual void onConfigChanged();
		virtual void onLowMemory();
		virtual void onSaveInstanceState();

		virtual long getMainLoopEventWaitTime();
		virtual void handleMainLoop(long bootTime);

	private:
		android_app* application;

		void processEvents(android_app *app);
		static void delegateAppCommand(struct android_app* app, int32_t command);
		static int32_t delegateInputEvent(struct android_app* app, AInputEvent* event);
};

#endif
