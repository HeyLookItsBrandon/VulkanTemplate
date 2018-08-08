#include "VulkanNativeApp.h"
#include "AndroidLogging.h"
#include "CapabilityUtils.h"
#include "MathUtils.h"
#include "AssetUtils.h"
#include <system_error>
#include <set>
#include <limits>

const std::vector<const char*> INSTANCE_EXTENSION_NAMES = {
		VK_KHR_SURFACE_EXTENSION_NAME,
		VK_KHR_ANDROID_SURFACE_EXTENSION_NAME,
		VK_EXT_DEBUG_REPORT_EXTENSION_NAME};

const std::vector<const char*> REQUIRED_DEVICE_EXTENSION_NAMES = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME };

// At the time of writing, these five layers make up the VK_LAYER_LUNARG_standard_validation meta
// layer, which according to presentation slides from LunarG isn't available on Android.
const std::vector<const char *> VALIDATION_LAYER_NAMES = {
		"VK_LAYER_GOOGLE_threading",
		"VK_LAYER_LUNARG_parameter_validation",
		"VK_LAYER_LUNARG_object_tracker",
		"VK_LAYER_LUNARG_core_validation",
		"VK_LAYER_GOOGLE_unique_objects"};

bool isDebugBuild() {
	bool debug = false;
    #ifndef NDEBUG
        debug = true;
    #endif

	return debug;
}

VulkanNativeApp::VulkanNativeApp(android_app* app) : BaseNativeApp(app), debug(isDebugBuild()) {
	InitVulkan();
}

void VulkanNativeApp::onWindowInitialized() {
	initializeDisplay();
}

void VulkanNativeApp::onWindowTerminated() {
	deinitializeDisplay();
}

void VulkanNativeApp::initializeDisplay() {
	if(debug) {
		logSupportedInstanceExtensions();
		logSupportedValidationLayers();
	}

	createInstance(instance);
	if(debug) {
		registerDebugReportCallback(instance, reportCallback);
	}

	surface = createSurface(instance);

	DeviceInfo deviceInfo = pickPhysicalDevice(surface);

	SwapChainSupportDetails swapChainDetails = {};
	swapChainDetails.format = pickFormat(deviceInfo.surfaceFormats);
	swapChainDetails.presentMode = pickPresentMode(deviceInfo.presentModes);
	VkSurfaceCapabilitiesKHR capabilities = getPhysicalDeviceSurfaceCapabilities(
			deviceInfo.physicalDevice, deviceInfo.surface);
	swapChainDetails.swapExtent = pickExtent(capabilities);
	swapChainDetails.imageCount = pickImageCount(capabilities);

	createLogicalDevice(deviceInfo, logicalDevice);
	vkGetDeviceQueue(logicalDevice, deviceInfo.queueFamilyIndex, 0, &graphicsQueue);
	vkGetDeviceQueue(logicalDevice, deviceInfo.presentationFamilyIndex, 0, &presentQueue);

	createSwapchain(
			swapchain,
			logicalDevice,
			swapChainDetails,
			deviceInfo,
			capabilities);

	createImageViews(swapChainDetails);
	createRenderPass(swapChainDetails);
	createGraphicsPipeline(swapChainDetails);
	createFramebuffers(swapChainDetails);
	createCommandPool(deviceInfo);
	createCommandBuffers(swapChainDetails);
	createSynchronizationStructures();

	setInitialized(true);
}

void VulkanNativeApp::deinitializeDisplay() {
	setInitialized(false);

	vkDeviceWaitIdle(logicalDevice);

	for(int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		vkDestroySemaphore(logicalDevice, renderCompletionSemaphores[i], nullptr);
		vkDestroySemaphore(logicalDevice, imageAvailabilitySemaphores[i], nullptr);
		vkDestroyFence(logicalDevice, inFlightFences[i], nullptr);
	}

	vkDestroyCommandPool(logicalDevice, commandPool, nullptr);

	for (auto framebuffer : swapchainFramebuffers) {
		vkDestroyFramebuffer(logicalDevice, framebuffer, nullptr);
	}

	vkDestroyPipeline(logicalDevice, graphicsPipeline, nullptr);
	vkDestroyPipelineLayout(logicalDevice, pipelineLayout, nullptr);
	vkDestroyRenderPass(logicalDevice, renderPass, nullptr);

	for(const VkImageView& view : swapchainImageViews) {
		vkDestroyImageView(logicalDevice, view, nullptr);
	}

	vkDestroySwapchainKHR(logicalDevice, swapchain, nullptr);

	vkDestroyDevice(logicalDevice, nullptr);

	if(debug) {
		destroyDebugReportCallback(instance, reportCallback, nullptr);
	}

	vkDestroySurfaceKHR(instance, surface, nullptr);
	vkDestroyInstance(instance, nullptr);
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

	createInfo.enabledExtensionCount = (uint32_t) INSTANCE_EXTENSION_NAMES.size();
	createInfo.ppEnabledExtensionNames = INSTANCE_EXTENSION_NAMES.data();

	if(debug) {
		createInfo.enabledLayerCount = (uint32_t) VALIDATION_LAYER_NAMES.size();
		createInfo.ppEnabledLayerNames = VALIDATION_LAYER_NAMES.data();
	} else {
		createInfo.enabledLayerCount = 0;
	}

	return createInfo;
}

void VulkanNativeApp::onReportingEvent(const char *message) {
	LOG_WARN("Validation message: %s", message);
}

VkDebugReportCallbackCreateInfoEXT VulkanNativeApp::createReportCallbackInfo() {
	VkDebugReportCallbackCreateInfoEXT createInfo = {};

	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
	createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT |
			VK_DEBUG_REPORT_WARNING_BIT_EXT |
			VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
	createInfo.pUserData = reinterpret_cast<void *>(this);
	createInfo.pfnCallback = delegateReportCallback;

	return createInfo;
}

VKAPI_ATTR VkBool32 VKAPI_CALL VulkanNativeApp::delegateReportCallback( VkDebugReportFlagsEXT flags,
		VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t code,
		const char* layerPrefix, const char* message, void* userData) {
	VulkanNativeApp *app = reinterpret_cast<VulkanNativeApp *>(userData);
	app->onReportingEvent(message);

	return (VkBool32) false;
}

const DeviceInfo VulkanNativeApp::pickPhysicalDevice(const VkSurfaceKHR& surface) {
	std::vector<VkPhysicalDevice> physicalDevices = getPhysicalDevices(instance);
	LOG_DEBUG("Found %lu physical devices.", physicalDevices.size());
	if (physicalDevices.size() == 0) {
		throw std::runtime_error("No physical devices found.");
	}

	for(const VkPhysicalDevice& physicalDevice : physicalDevices) {
		if(debug) {
			logPhysicalDeviceExtensionProperties(physicalDevice);
		}

		DeviceInfo info = {};
		info.surface = surface;
		info.surfaceFormats = getPhysicalDeviceSurfaceFormats(physicalDevice, surface);
		info.presentModes = getPhysicalDeviceSurfacePresentModes(physicalDevice, surface);

		if(!getPhysicalDeviceFeatures(physicalDevice).geometryShader ||
				!arePhysicalDeviceExtensionSupported(physicalDevice, REQUIRED_DEVICE_EXTENSION_NAMES) ||
				info.surfaceFormats.empty() ||
				info.presentModes.empty()) {
			continue;
		}

		std::vector<VkQueueFamilyProperties> queueFamilyProperties = getQueueFamilyProperties(physicalDevice);
		for(uint32_t i = 0; i < queueFamilyProperties.size(); i++) {
			VkQueueFamilyProperties familyProperties = queueFamilyProperties[i];
			if(familyProperties.queueCount > 0) {
				if(familyProperties.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
					info.queueFamilyIndex = i;
				}

				if(isPresentationSupported(physicalDevice, i, surface)) {
					info.presentationFamilyIndex = i;
				}
			}

			if(info.isComplete()) {
				info.physicalDevice = physicalDevice;
				return info;
			}
		}
	}

	throw std::runtime_error("No suitable physical devices found.");
}

std::vector<VkDeviceQueueCreateInfo> VulkanNativeApp::createQueueCreationInfos(DeviceInfo info) {
	std::set<unsigned int> uniqueQueueFamilies = {info.queueFamilyIndex, info.presentationFamilyIndex};
	std::vector<VkDeviceQueueCreateInfo> infos;

	for(unsigned int familyIndex : uniqueQueueFamilies) {
		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;

		queueCreateInfo.queueFamilyIndex = familyIndex;
		queueCreateInfo.queueCount = 1;

		float priorities[] = { 1.0f };
		queueCreateInfo.pQueuePriorities = &priorities[0];

		infos.push_back(queueCreateInfo);
	}

	return infos;
}

void VulkanNativeApp::createLogicalDevice(const DeviceInfo &deviceInfo, VkDevice& logicalDevice) {
	std::vector<VkDeviceQueueCreateInfo> queueCreationInfos = createQueueCreationInfos(deviceInfo);

	VkDeviceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreationInfos.size());
	createInfo.pQueueCreateInfos = queueCreationInfos.data();

	createInfo.enabledExtensionCount = static_cast<uint32_t>(REQUIRED_DEVICE_EXTENSION_NAMES.size());
	createInfo.ppEnabledExtensionNames = REQUIRED_DEVICE_EXTENSION_NAMES.data();

	if(debug) {
		createInfo.enabledLayerCount = (uint32_t) VALIDATION_LAYER_NAMES.size();
		createInfo.ppEnabledLayerNames = VALIDATION_LAYER_NAMES.data();
	} else {
		createInfo.enabledLayerCount = 0;
	}

	VkResult deviceCreationResult = vkCreateDevice(deviceInfo.physicalDevice, &createInfo, nullptr, &logicalDevice);
	assertSuccess(deviceCreationResult, "Failed to create logical device.");
}

VkSurfaceKHR VulkanNativeApp::createSurface(VkInstance& instance) {
	VkAndroidSurfaceCreateInfoKHR surfaceInfo = {};
	surfaceInfo.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
	surfaceInfo.window = getApplication()->window;

	VkSurfaceKHR surface;
	VkResult result = vkCreateAndroidSurfaceKHR(instance, &surfaceInfo, nullptr, &surface);
	assertSuccess(result, "Failed to create window");

	return surface;
}

void VulkanNativeApp::createInstance(VkInstance& instance) {
	VkApplicationInfo appInfo = createApplicationInfo();
	VkInstanceCreateInfo instanceCreationInfo = createInstanceCreationInfo(appInfo);
	VkResult instanceCreationResult = vkCreateInstance(&instanceCreationInfo, nullptr, &instance);
	LOG_DEBUG("Vulkan instance creation result: %d", instanceCreationResult);
	assertSuccess(instanceCreationResult, "Vulkan instance creation unsuccessful.");
}

void VulkanNativeApp::registerDebugReportCallback(VkInstance &instance,
		VkDebugReportCallbackEXT &reportCallback) {
	VkDebugReportCallbackCreateInfoEXT reportCallbackCreationInfo = createReportCallbackInfo();
	VkResult reportCallbackCreationResult = createDebugReportCallback(
			instance, &reportCallbackCreationInfo, nullptr, &reportCallback);
	LOG_DEBUG("Vulkan report callback creation result: %d", reportCallbackCreationResult);
	assertSuccess(reportCallbackCreationResult, "Failed to debug create report callback.");
}

VkSurfaceFormatKHR VulkanNativeApp::pickFormat(const std::vector<VkSurfaceFormatKHR>& formats) {
	if(formats.size() == 1 && formats[0].format == VK_FORMAT_UNDEFINED) {
		return {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
	}

	for(const VkSurfaceFormatKHR & format : formats) {
		if (format.format == VK_FORMAT_B8G8R8A8_UNORM &&
				format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return format;
		}
	}

	return formats[0]; // "Okay, I give up. Let's give this a try."
}

VkPresentModeKHR VulkanNativeApp::pickPresentMode(const std::vector<VkPresentModeKHR>& presentModes) {
	VkPresentModeKHR bestMode = VK_PRESENT_MODE_FIFO_KHR;

	for (const VkPresentModeKHR& presentMode : presentModes) {
		if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
			return presentMode;
		} else if (presentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
			bestMode = presentMode;
		}
	}

	return bestMode;
}

VkExtent2D VulkanNativeApp::pickExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
	if (capabilities.currentExtent.width == std::numeric_limits<uint32_t>::max()) {
		return {
				clamp(extentWidth, capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
				clamp(extentHeight, capabilities.minImageExtent.height, capabilities.maxImageExtent.height)};
	} else {
		return capabilities.currentExtent;
	}
}

int VulkanNativeApp::pickImageCount(VkSurfaceCapabilitiesKHR capabilities) {
	uint32_t desiredCount = capabilities.minImageCount + 1;
	return capabilities.maxImageCount == 0 ? // 0 means no limit
			desiredCount :
			std::min(desiredCount, capabilities.maxImageCount);
}

void VulkanNativeApp::createSwapchain(
		VkSwapchainKHR& swapchain,
		const VkDevice& device,
		const SwapChainSupportDetails &swapChainSupportDetails,
		const DeviceInfo &deviceInfo,
		const VkSurfaceCapabilitiesKHR &capabilities) {
	VkSwapchainCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;

	createInfo.surface = surface;

	createInfo.minImageCount = swapChainSupportDetails.imageCount;
	createInfo.imageFormat = swapChainSupportDetails.format.format;
	createInfo.imageColorSpace = swapChainSupportDetails.format.colorSpace;
	createInfo.imageExtent = swapChainSupportDetails.swapExtent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	if (deviceInfo.queueFamilyIndex == deviceInfo.presentationFamilyIndex) {
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	} else {
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;

		uint32_t indices[] = {
				(uint32_t) deviceInfo.queueFamilyIndex,
				(uint32_t) deviceInfo.presentationFamilyIndex};
		createInfo.pQueueFamilyIndices = indices;
	}

	createInfo.preTransform = capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
	createInfo.presentMode = swapChainSupportDetails.presentMode;
	createInfo.clipped = VK_TRUE;

	createInfo.oldSwapchain = VK_NULL_HANDLE;

	VkResult result = vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapchain);
	assertSuccess(result, "Failed to create swap chain.");

	getSwapchainImages(device, swapchain, swapchainImages);
}

void VulkanNativeApp::handleMainLoop(long bootTime) {
	if(initialized) {
		drawFrame();
	}
}

void VulkanNativeApp::setInitialized(bool initialized) {
	this->initialized = initialized;
	setMainLoopEventWaitTime(initialized ? 0  : -1);
}

void VulkanNativeApp::createImageViews(const SwapChainSupportDetails& swapChainSupportDetails) {
	swapchainImageViews.resize(swapchainImages.size());
	for (size_t i = 0; i < swapchainImages.size(); i++) {
		VkImageViewCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = swapchainImages[i];
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = swapChainSupportDetails.format.format;
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		VkResult result = vkCreateImageView(logicalDevice, &createInfo, nullptr, &swapchainImageViews[i]);
		assertSuccess(result, "Failed to create image view.");
	}
}

void VulkanNativeApp::createGraphicsPipeline(SwapChainSupportDetails swapChainDetails) {
	std::vector<char> vertexShaderBytecode = readAsset(
			getAssetManager(), "shaders/shader_base.vert.spv");
	VkShaderModule vertexShaderModule = createShaderModule(logicalDevice, vertexShaderBytecode);

	std::vector<char> fragmentShaderBytecode = readAsset(
			getAssetManager(), "shaders/shader_base.frag.spv");
	VkShaderModule fragmentShaderModule = createShaderModule(logicalDevice, fragmentShaderBytecode);

	VkPipelineShaderStageCreateInfo vertexShaderStageInfo = {};
	vertexShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertexShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertexShaderStageInfo.module = vertexShaderModule;
	vertexShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragmentShaderStageInfo = {};
	fragmentShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragmentShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragmentShaderStageInfo.module = fragmentShaderModule;
	fragmentShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStages[] = {vertexShaderStageInfo, fragmentShaderStageInfo};

	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 0;
	vertexInputInfo.vertexAttributeDescriptionCount = 0;

	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float) swapChainDetails.swapExtent.width;
	viewport.height = (float)swapChainDetails.swapExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor = {};
	scissor.offset = {0, 0};
	scissor.extent = swapChainDetails.swapExtent;

	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	VkPipelineRasterizationStateCreateInfo rasterizer = {};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;

	VkPipelineMultisampleStateCreateInfo multisampling = {};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;

	VkPipelineColorBlendStateCreateInfo colorBlending = {};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f; // Optional
	colorBlending.blendConstants[1] = 0.0f; // Optional
	colorBlending.blendConstants[2] = 0.0f; // Optional
	colorBlending.blendConstants[3] = 0.0f; // Optional

	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	assertSuccess(vkCreatePipelineLayout(logicalDevice, &pipelineLayoutInfo, nullptr, &pipelineLayout),
			"Failed to create pipeline layout.");

	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;

	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = nullptr; // Optional
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = nullptr; // Optional
	pipelineInfo.layout = pipelineLayout;
	pipelineInfo.renderPass = renderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
	pipelineInfo.basePipelineIndex = -1; // Optional

	assertSuccess(vkCreateGraphicsPipelines(logicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline),
			"failed to create graphics pipeline!");

	vkDestroyShaderModule(logicalDevice, fragmentShaderModule, nullptr);
	vkDestroyShaderModule(logicalDevice, vertexShaderModule, nullptr);
}

void VulkanNativeApp::createFramebuffers(const SwapChainSupportDetails &swapChainSupportDetails) {
	swapchainFramebuffers.resize(swapchainImageViews.size());

	for (size_t i = 0; i < swapchainImageViews.size(); i++) {
		VkImageView attachments[] = { swapchainImageViews[i] };

		VkFramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = attachments;
		framebufferInfo.width = swapChainSupportDetails.swapExtent.width;
		framebufferInfo.height = swapChainSupportDetails.swapExtent.height;
		framebufferInfo.layers = 1;

		assertSuccess(vkCreateFramebuffer(logicalDevice, &framebufferInfo, nullptr,
				&swapchainFramebuffers[i]), "Failed to create framebuffer");
	}
}

void VulkanNativeApp::createCommandPool(const DeviceInfo &deviceInfo) {
	VkCommandPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = deviceInfo.queueFamilyIndex;
	poolInfo.flags = 0; // Optional

	assertSuccess(vkCreateCommandPool(logicalDevice, &poolInfo, nullptr, &commandPool),
			"Failed to create command pool.");
}

void VulkanNativeApp::createCommandBuffers(const SwapChainSupportDetails &swapChainSupportDetails) {
	commandBuffers.resize(swapchainFramebuffers.size());

	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = (uint32_t) commandBuffers.size();

	assertSuccess(vkAllocateCommandBuffers(logicalDevice, &allocInfo, commandBuffers.data()),
			"Failed to allocate command buffers.");

	for (size_t i = 0; i < commandBuffers.size(); i++) {
		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
		beginInfo.pInheritanceInfo = nullptr; // Optional

		assertSuccess(vkBeginCommandBuffer(commandBuffers[i], &beginInfo),
				"Failed to begin recording command buffer!");

		VkRenderPassBeginInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = renderPass;
		renderPassInfo.framebuffer = swapchainFramebuffers[i];
		renderPassInfo.renderArea.offset = {0, 0};
		renderPassInfo.renderArea.extent = swapChainSupportDetails.swapExtent;

		VkClearValue clearColor = {0.0f, 0.0f, 0.0f, 1.0f};
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearColor;

		vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
		vkCmdDraw(commandBuffers[i], 3, 1, 0, 0);
		vkCmdEndRenderPass(commandBuffers[i]);

		assertSuccess(vkEndCommandBuffer(commandBuffers[i]), "Failed to record command buffer.");
	}
}

void VulkanNativeApp::createSynchronizationStructures() {
	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceCreationInfo = {};
	fenceCreationInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceCreationInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	imageAvailabilitySemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	renderCompletionSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

	for(int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		assertSuccess(vkCreateSemaphore(logicalDevice, &semaphoreInfo, nullptr, &imageAvailabilitySemaphores[i]),
				"Failed to create semaphore.");
		assertSuccess(vkCreateSemaphore(logicalDevice, &semaphoreInfo, nullptr, &renderCompletionSemaphores[i]),
				"Failed to create semaphore.");
		assertSuccess(vkCreateFence(logicalDevice, &fenceCreationInfo, nullptr, &inFlightFences[i]),
				"Failed to create fence.");
	}
}

void VulkanNativeApp::createRenderPass(SwapChainSupportDetails swapChainDetails) {
	colorAttachment = {};
	colorAttachment.format = swapChainDetails.format.format;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentReference = {};
	colorAttachmentReference.attachment = 0;
	colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	// An array. Location 0 in fragment shader is a reference to this!
	subpass.pColorAttachments = &colorAttachmentReference;

	VkSubpassDependency dependency = {};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colorAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	assertSuccess(vkCreateRenderPass(logicalDevice, &renderPassInfo, nullptr, &renderPass),
			"Failed to create render pass.");
}

void VulkanNativeApp::drawFrame() {
	vkWaitForFences(logicalDevice, 1, &inFlightFences[frameNumber], VK_TRUE, std::numeric_limits<uint64_t>::max());
	vkResetFences(logicalDevice, 1, &inFlightFences[frameNumber]);

	uint32_t imageIndex;
	vkAcquireNextImageKHR(logicalDevice, swapchain, std::numeric_limits<uint64_t>::max(),
			imageAvailabilitySemaphores[frameNumber], VK_NULL_HANDLE, &imageIndex);

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[] = {imageAvailabilitySemaphores[frameNumber]};
	VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;

	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffers[imageIndex];

	VkSemaphore signalSemaphores[] = {renderCompletionSemaphores[frameNumber]};
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	assertSuccess(vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[frameNumber]),
			"Failed to submit draw command buffer.");

	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapChains[] = {swapchain};
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;

	presentInfo.pImageIndices = &imageIndex;

	vkQueuePresentKHR(presentQueue, &presentInfo);

	frameNumber = (frameNumber + 1) % MAX_FRAMES_IN_FLIGHT;
}
