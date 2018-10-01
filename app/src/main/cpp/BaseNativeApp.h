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

		virtual void beforeMainLoop();
		virtual void handleMainLoop(long bootTime);
		virtual void afterMainLoop();

		int getMainLoopEventWaitTime();

		/**
		 * Sets how long the main loop should wait for Looper events to be processed. A value of -1
		 * will block indefinitely and 0 will continue immediately. This call has the side effect of
		 * waking the main looper.
		 *
		 * @param timeout The timeout in milliseconds.
		 */
		void setMainLoopEventWaitTime(int timeout);

	private:
		android_app* application;
		int mainLoopEventWaitTime = -1;

		static void delegateAppCommand(struct android_app* app, int32_t command);
		static int32_t delegateInputEvent(struct android_app* app, AInputEvent* event);
};

#endif
