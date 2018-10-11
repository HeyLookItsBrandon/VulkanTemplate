#ifndef VULKANNATIVEAPP_H
#define VULKANNATIVEAPP_H

#include "BaseNativeApp.h"
#include "vulkan_wrapper/vulkan_wrapper.h"

#include <vector>
#include <array>
#include <tuple>
#include <memory>
#include "glm/glm.hpp"

struct Vertex {
	glm::vec2 position;
	glm::vec3 color;

	static VkVertexInputBindingDescription getBindingDescription() {
		VkVertexInputBindingDescription bindingDescription = {};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescription;
	}

	static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions() {
		std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions = {};

		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[0].offset = (uint32_t) offsetof(Vertex, position);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = (uint32_t) offsetof(Vertex, color);

		return attributeDescriptions;
	}
};

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
		const u_long MAX_FRAMES_IN_FLIGHT = 2;

		VkInstance instance = {};
		VkDebugReportCallbackEXT reportCallback = {};
		VkSurfaceKHR surface;
		VkDevice device = {};
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
		VkBuffer vertexBuffer;
		VkDeviceMemory vertexBufferMemory;
		VkBuffer indexBuffer;
		VkDeviceMemory indexBufferMemory;
		VkCommandPool commandPool;
		std::vector<VkCommandBuffer> commandBuffers;

		std::vector<VkSemaphore> imageAvailabilitySemaphores;
		std::vector<VkSemaphore> renderCompletionSemaphores;
		std::vector<VkFence> inFlightFences;
		u_long frameNumber = 0;

		bool framebufferResized = false;

		VkAttachmentDescription colorAttachment;
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

		uint32_t pickImageCount(VkSurfaceCapabilitiesKHR capabilities);

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

		void createBuffer(const VkPhysicalDeviceMemoryProperties &memoryProperties,
				VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
				VkBuffer& buffer, VkDeviceMemory& bufferMemory);
		void createVertexBuffer(const VkPhysicalDeviceMemoryProperties &memoryProperties);
		void createIndexBuffer(const VkPhysicalDeviceMemoryProperties &memoryProperties);
		void copyBuffer(VkBuffer sourceBuffer, VkBuffer destinationBuffer, VkDeviceSize size);
		uint32_t pickMemoryTypeIndex(const VkPhysicalDeviceMemoryProperties &memoryProperties,
				uint32_t requiredMemoryTypeBits,
				VkMemoryPropertyFlags requiredProperties);
};

#endif
