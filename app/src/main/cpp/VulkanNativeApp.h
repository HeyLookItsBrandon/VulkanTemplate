#ifndef VULKANNATIVEAPP_H
#define VULKANNATIVEAPP_H

#include "BaseNativeApp.h"
#include "vulkan_wrapper/vulkan_wrapper.h"

#include <vector>
#include <tuple>
#include <memory>

struct DeviceInfo {
	const static unsigned int NONE = static_cast<const unsigned int>(-1);

	VkPhysicalDevice physicalDevice;
	VkSurfaceKHR  surface;
	unsigned int queueFamilyIndex = NONE;
	unsigned int presentationFamilyIndex = NONE;
	std::vector<VkSurfaceFormatKHR> surfaceFormats;
	std::vector<VkPresentModeKHR> presentModes;

	bool isComplete() {
		return queueFamilyIndex != NONE && presentationFamilyIndex != NONE;
	}
};

struct SwapChainSupportDetails {
	VkSurfaceFormatKHR format;
	VkPresentModeKHR presentMode;
	VkExtent2D swapExtent;
	uint32_t imageCount;
};

class VulkanNativeApp : public BaseNativeApp {
	public:
		VulkanNativeApp(android_app* app);
	protected:
		void initializeDisplay();
		void deinitializeDisplay();

		void onWindowInitialized() override;
		void onWindowTerminated() override;
		void onWindowResized() override;
		void handleMainLoop(long bootTime) override;

		virtual void onReportingEvent(const char *message);

	private:
		const bool debug;
		const int MAX_FRAMES_IN_FLIGHT = 2;

		VkInstance instance = {};
		VkDebugReportCallbackEXT reportCallback = {};
		VkSurfaceKHR surface;
		VkDevice logicalDevice = {};
		VkQueue graphicsQueue;
		VkQueue presentQueue;
		VkSwapchainKHR swapchain;
		SwapChainSupportDetails swapchainDetails;
		DeviceInfo deviceInfo;
		VkSurfaceCapabilitiesKHR surfaceCapabilities;
		std::vector<VkImage> swapchainImages;
		std::vector<VkImageView> swapchainImageViews;
		VkRenderPass renderPass;
		VkPipelineLayout pipelineLayout;
		VkPipeline graphicsPipeline;
		std::vector<VkFramebuffer> swapchainFramebuffers;
		VkCommandPool commandPool;
		std::vector<VkCommandBuffer> commandBuffers;

		std::vector<VkSemaphore> imageAvailabilitySemaphores;
		std::vector<VkSemaphore> renderCompletionSemaphores;
		std::vector<VkFence> inFlightFences;
		unsigned int frameNumber = 0;

		bool framebufferResized = false;

		VkAttachmentDescription colorAttachment;
		VkSubpassDescription subpass;
		bool initialized = false;

		void setInitialized(bool initialized);

		void createInstance(VkInstance& instance);
		void registerDebugReportCallback(VkInstance &instance,
				VkDebugReportCallbackEXT &reportCallback);

		VkApplicationInfo createApplicationInfo();
		VkInstanceCreateInfo createInstanceCreationInfo(VkApplicationInfo& applicationInfo);
		VkDebugReportCallbackCreateInfoEXT createReportCallbackInfo();
		std::vector<VkDeviceQueueCreateInfo> createQueueCreationInfos(DeviceInfo info);

		const DeviceInfo pickPhysicalDevice(const VkSurfaceKHR& surface);

		void createLogicalDevice(const DeviceInfo &deviceInfo, VkDevice& logicalDevice);

		VkSurfaceKHR createSurface(VkInstance& instance);

		VkSurfaceFormatKHR pickFormat(const std::vector<VkSurfaceFormatKHR>& formats);

		static VKAPI_ATTR VkBool32 VKAPI_CALL delegateReportCallback( VkDebugReportFlagsEXT flags,
				VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location,
				int32_t code, const char* layerPrefix, const char* message, void* userData);

		VkPresentModeKHR pickPresentMode(const std::vector<VkPresentModeKHR> &presentModes);

		VkExtent2D pickExtent(const VkSurfaceCapabilitiesKHR &capabilities);

		int pickImageCount(VkSurfaceCapabilitiesKHR capabilities);

		void createSwapchain(
				VkSwapchainKHR& swapchain,
				const VkDevice& device,
				const SwapChainSupportDetails &swapChainSupportDetails,
				const DeviceInfo &deviceInfo,
				const VkSurfaceCapabilitiesKHR &capabilities);

		void createImageViews(const SwapChainSupportDetails& swapChainSupportDetails);

		void createRenderPass(SwapChainSupportDetails swapchainDetails);
		void createGraphicsPipeline(SwapChainSupportDetails swapChainDetails);
		void createFramebuffers(const SwapChainSupportDetails &swapChainSupportDetails);
		void createCommandPool(const DeviceInfo &deviceInfo);
		void createCommandBuffers(const SwapChainSupportDetails &swapChainSupportDetails);
		void createSynchronizationStructures();

		void drawFrame();

		void cleanupSwapchain();
		void recreateSwapchain();
};

#endif
