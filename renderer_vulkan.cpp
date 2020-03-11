// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include <vulkan/vulkan.h>
#include <stdio.h>

struct DepthStencil
{
    VkImage image = VK_NULL_HANDLE;
    VkDeviceMemory mem = VK_NULL_HANDLE;
    VkImageView view = VK_NULL_HANDLE;
};

struct SwapchainResource
{
    VkImage image = VK_NULL_HANDLE;
    VkImageView view = VK_NULL_HANDLE;
    VkCommandBuffer drawCommandBuffer = VK_NULL_HANDLE;
    VkSemaphore renderCompleteSemaphore = VK_NULL_HANDLE;
    VkSemaphore imageAcquiredSemaphore = VK_NULL_HANDLE;
    VkFence fence = VK_NULL_HANDLE;
    VkFramebuffer frameBuffer = VK_NULL_HANDLE;
    DepthStencil depthStencil;
};

struct Raytracer
{
	VkBuffer rayGenBindingTable = VK_NULL_HANDLE;
    VkDeviceMemory rayGenBindingMemory = VK_NULL_HANDLE;
	VkBuffer rayMissBindingTable = VK_NULL_HANDLE;
    VkDeviceMemory rayMissBindingMemory = VK_NULL_HANDLE;
	VkBuffer rayHitBindingTable = VK_NULL_HANDLE;
    VkDeviceMemory rayHitBindingMemory = VK_NULL_HANDLE;
	VkBuffer rayCallableBindingTable = VK_NULL_HANDLE;
    VkDeviceMemory rayCallableBindingMemory = VK_NULL_HANDLE;
};

struct ShaderBindingTable
{
	const unsigned groupCount = 1;
	VkRayTracingShaderGroupCreateInfoNV groups[ 1 ] = {};
};

struct Renderer
{
    VkDevice device = VK_NULL_HANDLE;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkInstance instance = VK_NULL_HANDLE;
    VkQueue graphicsQueue = VK_NULL_HANDLE;
    uint32_t swapchainImageCount = 0;
    VkPhysicalDeviceProperties properties = {};
    VkPhysicalDeviceFeatures features = {};
    VkFormat colorFormat = VK_FORMAT_UNDEFINED;
    VkPhysicalDeviceMemoryProperties deviceMemoryProperties;
    VkDebugUtilsMessengerEXT dbgMessenger = VK_NULL_HANDLE;
    SwapchainResource swapchainResources[ 4 ] = {};
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    VkSwapchainKHR swapchain = VK_NULL_HANDLE;
    uint32_t graphicsQueueNodeIndex = 0;
    VkCommandPool cmdPool;
    uint32_t currentBuffer = 0;
    unsigned frameIndex = 0;
    int width = 0;
    int height = 0;
    VkPipelineShaderStageCreateInfo rayHitInfo = {};
    VkShaderModule rayHitModule = VK_NULL_HANDLE;
    VkShaderModule rayGenModule = VK_NULL_HANDLE;
    VkShaderModule rayMissModule = VK_NULL_HANDLE;
	Raytracer raytracer;
	VkPipeline psoRayHit = VK_NULL_HANDLE;
	VkPipeline psoRayGen = VK_NULL_HANDLE;
	VkPipeline psoRayMiss = VK_NULL_HANDLE;
	VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
	ShaderBindingTable sbt;
	VkImage outputImage = VK_NULL_HANDLE;
	VkImageView outputImageView = VK_NULL_HANDLE;
	VkDeviceMemory outputImageMemory = VK_NULL_HANDLE;
	VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
	VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
	VkSampler sampler = VK_NULL_HANDLE;
} gRenderer;

PFN_vkCreateSwapchainKHR createSwapchainKHR;
PFN_vkGetSwapchainImagesKHR getSwapchainImagesKHR;
PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR getPhysicalDeviceSurfaceCapabilitiesKHR;
PFN_vkGetPhysicalDeviceSurfacePresentModesKHR getPhysicalDeviceSurfacePresentModesKHR;
PFN_vkGetPhysicalDeviceSurfaceSupportKHR getPhysicalDeviceSurfaceSupportKHR;
PFN_vkGetPhysicalDeviceSurfaceFormatsKHR getPhysicalDeviceSurfaceFormatsKHR;
PFN_vkAcquireNextImageKHR acquireNextImageKHR;
PFN_vkQueuePresentKHR queuePresentKHR;
PFN_vkSetDebugUtilsObjectNameEXT setDebugUtilsObjectNameEXT;
PFN_vkCmdBeginDebugUtilsLabelEXT CmdBeginDebugUtilsLabelEXT;
PFN_vkCmdEndDebugUtilsLabelEXT CmdEndDebugUtilsLabelEXT;
PFN_vkCmdTraceRaysNV CmdTraceRaysNV;
PFN_vkCreateRayTracingPipelinesNV CreateRayTracingPipelinesNV;
PFN_vkCmdBuildAccelerationStructureNV CmdBuildAccelerationStructureNV;
PFN_vkCreateAccelerationStructureNV CreateAccelerationStructureNV;

void SetObjectName( VkDevice device, uint64_t object, VkObjectType objectType, const char* name )
{
    if (setDebugUtilsObjectNameEXT)
    {
        VkDebugUtilsObjectNameInfoEXT nameInfo = {};
        nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
        nameInfo.objectType = objectType;
        nameInfo.objectHandle = object;
        nameInfo.pObjectName = name;
        setDebugUtilsObjectNameEXT( device, &nameInfo );
    }
}

const char* getObjectType( VkObjectType type )
{
    switch( type )
    {
    case VK_OBJECT_TYPE_QUERY_POOL: return "VK_OBJECT_TYPE_QUERY_POOL";
    case VK_OBJECT_TYPE_OBJECT_TABLE_NVX: return "VK_OBJECT_TYPE_OBJECT_TABLE_NVX";
    case VK_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION: return "VK_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION";
    case VK_OBJECT_TYPE_SEMAPHORE: return "VK_OBJECT_TYPE_SEMAPHORE";
    case VK_OBJECT_TYPE_SHADER_MODULE: return "VK_OBJECT_TYPE_SHADER_MODULE";
    case VK_OBJECT_TYPE_SWAPCHAIN_KHR: return "VK_OBJECT_TYPE_SWAPCHAIN_KHR";
    case VK_OBJECT_TYPE_SAMPLER: return "VK_OBJECT_TYPE_SAMPLER";
    case VK_OBJECT_TYPE_INDIRECT_COMMANDS_LAYOUT_NVX: return "VK_OBJECT_TYPE_INDIRECT_COMMANDS_LAYOUT_NVX";
    case VK_OBJECT_TYPE_DEBUG_REPORT_CALLBACK_EXT: return "VK_OBJECT_TYPE_DEBUG_REPORT_CALLBACK_EXT";
    case VK_OBJECT_TYPE_IMAGE: return "VK_OBJECT_TYPE_IMAGE";
    case VK_OBJECT_TYPE_UNKNOWN: return "VK_OBJECT_TYPE_UNKNOWN";
    case VK_OBJECT_TYPE_DESCRIPTOR_POOL: return "VK_OBJECT_TYPE_DESCRIPTOR_POOL";
    case VK_OBJECT_TYPE_COMMAND_BUFFER: return "VK_OBJECT_TYPE_COMMAND_BUFFER";
    case VK_OBJECT_TYPE_BUFFER: return "VK_OBJECT_TYPE_BUFFER";
    case VK_OBJECT_TYPE_SURFACE_KHR: return "VK_OBJECT_TYPE_SURFACE_KHR";
    case VK_OBJECT_TYPE_INSTANCE: return "VK_OBJECT_TYPE_INSTANCE";
    case VK_OBJECT_TYPE_VALIDATION_CACHE_EXT: return "VK_OBJECT_TYPE_VALIDATION_CACHE_EXT";
    case VK_OBJECT_TYPE_IMAGE_VIEW: return "VK_OBJECT_TYPE_IMAGE_VIEW";
    case VK_OBJECT_TYPE_DESCRIPTOR_SET: return "VK_OBJECT_TYPE_DESCRIPTOR_SET";
    case VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT: return "VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT";
    case VK_OBJECT_TYPE_COMMAND_POOL: return "VK_OBJECT_TYPE_COMMAND_POOL";
    case VK_OBJECT_TYPE_PHYSICAL_DEVICE: return "VK_OBJECT_TYPE_PHYSICAL_DEVICE";
    case VK_OBJECT_TYPE_DISPLAY_KHR: return "VK_OBJECT_TYPE_DISPLAY_KHR";
    case VK_OBJECT_TYPE_BUFFER_VIEW: return "VK_OBJECT_TYPE_BUFFER_VIEW";
    case VK_OBJECT_TYPE_DEBUG_UTILS_MESSENGER_EXT: return "VK_OBJECT_TYPE_DEBUG_UTILS_MESSENGER_EXT";
    case VK_OBJECT_TYPE_FRAMEBUFFER: return "VK_OBJECT_TYPE_FRAMEBUFFER";
    case VK_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE: return "VK_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE";
    case VK_OBJECT_TYPE_PIPELINE_CACHE: return "VK_OBJECT_TYPE_PIPELINE_CACHE";
    case VK_OBJECT_TYPE_PIPELINE_LAYOUT: return "VK_OBJECT_TYPE_PIPELINE_LAYOUT";
    case VK_OBJECT_TYPE_DEVICE_MEMORY: return "VK_OBJECT_TYPE_DEVICE_MEMORY";
    case VK_OBJECT_TYPE_FENCE: return "VK_OBJECT_TYPE_FENCE";
    case VK_OBJECT_TYPE_QUEUE: return "VK_OBJECT_TYPE_QUEUE";
    case VK_OBJECT_TYPE_DEVICE: return "VK_OBJECT_TYPE_DEVICE";
    case VK_OBJECT_TYPE_RENDER_PASS: return "VK_OBJECT_TYPE_RENDER_PASS";
    case VK_OBJECT_TYPE_DISPLAY_MODE_KHR: return "VK_OBJECT_TYPE_DISPLAY_MODE_KHR";
    case VK_OBJECT_TYPE_EVENT: return "VK_OBJECT_TYPE_EVENT";
    case VK_OBJECT_TYPE_PIPELINE: return "VK_OBJECT_TYPE_PIPELINE";
    default: return "unhandled type";
    }
}

VKAPI_ATTR VkBool32 VKAPI_CALL dbgFunc( VkDebugUtilsMessageSeverityFlagBitsEXT msgSeverity, VkDebugUtilsMessageTypeFlagsEXT msgType,
                                        const VkDebugUtilsMessengerCallbackDataEXT* callbackData, void* /*userData*/ )
{
    if (msgSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
    {
        printf( "ERROR: %s\n", callbackData->pMessage );
    }
    else if (msgSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
    {
        printf( "WARNING: %s\n", callbackData->pMessage );
    }
    else if (msgSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
    {
        printf( "INFO: %s\n", callbackData->pMessage );
    }
    else if (msgSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
    {
        printf( "VERBOSE: %s\n", callbackData->pMessage );
    }

    if (msgType & VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT)
    {
        printf( "GENERAL: %s\n", callbackData->pMessage );
    }
    else if (msgType & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT)
    {
        printf( "PERF: %s\n", callbackData->pMessage );
    }

    if (callbackData->objectCount > 0)
    {
        printf( "Objects: %u\n", callbackData->objectCount );

        for (uint32_t i = 0; i < callbackData->objectCount; ++i)
        {
            const char* name = callbackData->pObjects[ i ].pObjectName ? callbackData->pObjects[ i ].pObjectName : "unnamed";
            printf( "Object %u: name: %s, type: %s\n", i, name, getObjectType( callbackData->pObjects[ i ].objectType ) );
        }
    }
    
    cassert( !"Vulkan debug message" );

    return VK_FALSE;
}

uint32_t GetMemoryType( uint32_t typeBits, const VkPhysicalDeviceMemoryProperties& deviceMemoryProperties, VkFlags properties )
{
    for (uint32_t i = 0; i < deviceMemoryProperties.memoryTypeCount; ++i)
    {
        if ((typeBits & (1 << i)) != 0 && (deviceMemoryProperties.memoryTypes[ i ].propertyFlags & properties) == properties)
        {
            return i;
        }
    }

    cassert( !"could not get memory type" );
    return 0;
}

void SetImageLayout( VkCommandBuffer cmdbuffer, VkImage image, VkImageAspectFlags aspectMask, VkImageLayout oldImageLayout,
                     VkImageLayout newImageLayout, unsigned layerCount, unsigned mipLevel, unsigned mipLevelCount, VkPipelineStageFlags srcStageFlags )
{
    VkImageMemoryBarrier imageMemoryBarrier = {};
    imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

    imageMemoryBarrier.oldLayout = oldImageLayout;
    imageMemoryBarrier.newLayout = newImageLayout;
    imageMemoryBarrier.image = image;
    imageMemoryBarrier.subresourceRange.aspectMask = aspectMask;
    imageMemoryBarrier.subresourceRange.baseMipLevel = mipLevel;
    imageMemoryBarrier.subresourceRange.levelCount = mipLevelCount;
    imageMemoryBarrier.subresourceRange.layerCount = layerCount;

    switch (oldImageLayout)
    {
    case VK_IMAGE_LAYOUT_UNDEFINED:
        imageMemoryBarrier.srcAccessMask = 0;
        break;
    case VK_IMAGE_LAYOUT_PREINITIALIZED:
        imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
        break;
    case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
        imageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        break;
    case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
        imageMemoryBarrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        break;
    case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
        imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        break;
    case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
        imageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
        break;
    default:
        break;
    }

    if (newImageLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    }

    if (newImageLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
    {
        imageMemoryBarrier.srcAccessMask = imageMemoryBarrier.srcAccessMask | VK_ACCESS_TRANSFER_READ_BIT;
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    }

    if (newImageLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
    {
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    }

    if (newImageLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
    {
        imageMemoryBarrier.dstAccessMask = imageMemoryBarrier.dstAccessMask | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    }

    if (newImageLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        if (imageMemoryBarrier.srcAccessMask == 0)
        {
            imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
        }

        if (srcStageFlags == VK_PIPELINE_STAGE_TRANSFER_BIT)
        {
            imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        }

        imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    }

    if (newImageLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
    {
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    }

    VkPipelineStageFlags destStageFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

    if (imageMemoryBarrier.dstAccessMask == VK_ACCESS_TRANSFER_WRITE_BIT)
    {
        destStageFlags = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
        
    if (imageMemoryBarrier.dstAccessMask == VK_ACCESS_SHADER_READ_BIT)
    {
        destStageFlags = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }

    if (imageMemoryBarrier.dstAccessMask == VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT)
    {
        destStageFlags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    }

    if (imageMemoryBarrier.dstAccessMask == VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT)
    {
        destStageFlags = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    }

    if (imageMemoryBarrier.dstAccessMask == VK_ACCESS_TRANSFER_READ_BIT)
    {
        destStageFlags = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }

    vkCmdPipelineBarrier( cmdbuffer, srcStageFlags, destStageFlags, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier );
}

void CreateInstance()
{
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "VulkanRaytracer";
    appInfo.pEngineName = "VulkanRaytracer";
    appInfo.apiVersion = VK_API_VERSION_1_1;

    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties( nullptr, &extensionCount, nullptr );
    cassert( extensionCount < 25 );
    VkExtensionProperties extensions[ 25 ];
    vkEnumerateInstanceExtensionProperties( nullptr, &extensionCount, extensions );

    bool hasDebugUtils = false;

#ifndef NDEBUG
    for (uint32_t i = 0; i < extensionCount; ++i)
    {
        hasDebugUtils |= cstrcmp( extensions[ i ].extensionName, VK_EXT_DEBUG_UTILS_EXTENSION_NAME ) == 0;
    }
#endif
    
    const char* enabledExtensions[] =
    {
        VK_KHR_SURFACE_EXTENSION_NAME,
        SURFACE_EXTENSION_NAME,
		VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME
    };

    VkInstanceCreateInfo instanceCreateInfo = {};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pApplicationInfo = &appInfo;
    instanceCreateInfo.enabledExtensionCount = hasDebugUtils ? 4 : 3;
    instanceCreateInfo.ppEnabledExtensionNames = enabledExtensions;
#if _DEBUG
    const char* validationLayerNames[] = { "VK_LAYER_KHRONOS_validation" };
    
    if (hasDebugUtils)
    {
        instanceCreateInfo.enabledLayerCount = 1;
        instanceCreateInfo.ppEnabledLayerNames = validationLayerNames;
    }
#endif
#if 0
    const VkValidationFeatureEnableEXT enables[] = { VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT, VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_RESERVE_BINDING_SLOT_EXT, VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT };
    VkValidationFeaturesEXT features = {};
    features.sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT;
    features.enabledValidationFeatureCount = 3;
    features.pEnabledValidationFeatures = enables;

    instanceCreateInfo.pNext = &features;
#endif
    VkResult result = vkCreateInstance( &instanceCreateInfo, nullptr, &gRenderer.instance );

    if (result != VK_SUCCESS)
    {
        printf( "Unable to create instance!\n" );
        return;
    }

#if _DEBUG
    if (hasDebugUtils)
    {
        PFN_vkCreateDebugUtilsMessengerEXT dbgCreateDebugUtilsMessenger = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr( gRenderer.instance, "vkCreateDebugUtilsMessengerEXT" );
        VkDebugUtilsMessengerCreateInfoEXT dbg_messenger_create_info = {};
        dbg_messenger_create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        dbg_messenger_create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        dbg_messenger_create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        dbg_messenger_create_info.pfnUserCallback = dbgFunc;
        result = dbgCreateDebugUtilsMessenger( gRenderer.instance, &dbg_messenger_create_info, nullptr, &gRenderer.dbgMessenger );

        CmdBeginDebugUtilsLabelEXT = (PFN_vkCmdBeginDebugUtilsLabelEXT)vkGetInstanceProcAddr( gRenderer.instance, "vkCmdBeginDebugUtilsLabelEXT" );
        CmdEndDebugUtilsLabelEXT = (PFN_vkCmdEndDebugUtilsLabelEXT)vkGetInstanceProcAddr( gRenderer.instance, "vkCmdEndDebugUtilsLabelEXT" );
    }
#endif
    cassert( result == VK_SUCCESS );
}

void LoadFunctionPointers()
{
    createSwapchainKHR = (PFN_vkCreateSwapchainKHR)vkGetDeviceProcAddr( gRenderer.device, "vkCreateSwapchainKHR" );
    getPhysicalDeviceSurfaceCapabilitiesKHR = (PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR)vkGetInstanceProcAddr( gRenderer.instance, "vkGetPhysicalDeviceSurfaceCapabilitiesKHR" );
    getPhysicalDeviceSurfacePresentModesKHR = (PFN_vkGetPhysicalDeviceSurfacePresentModesKHR)vkGetInstanceProcAddr( gRenderer.instance, "vkGetPhysicalDeviceSurfacePresentModesKHR" );
    getPhysicalDeviceSurfaceFormatsKHR = (PFN_vkGetPhysicalDeviceSurfaceFormatsKHR)vkGetInstanceProcAddr( gRenderer.instance, "vkGetPhysicalDeviceSurfaceFormatsKHR" );
    getPhysicalDeviceSurfaceSupportKHR = (PFN_vkGetPhysicalDeviceSurfaceSupportKHR)vkGetInstanceProcAddr( gRenderer.instance, "vkGetPhysicalDeviceSurfaceSupportKHR" );
    getSwapchainImagesKHR = (PFN_vkGetSwapchainImagesKHR)vkGetDeviceProcAddr( gRenderer.device, "vkGetSwapchainImagesKHR" );
    acquireNextImageKHR = (PFN_vkAcquireNextImageKHR)vkGetDeviceProcAddr( gRenderer.device, "vkAcquireNextImageKHR" );
    queuePresentKHR = (PFN_vkQueuePresentKHR)vkGetDeviceProcAddr( gRenderer.device, "vkQueuePresentKHR" );
	setDebugUtilsObjectNameEXT = (PFN_vkSetDebugUtilsObjectNameEXT)vkGetInstanceProcAddr( gRenderer.instance, "vkSetDebugUtilsObjectNameEXT" );
	CmdTraceRaysNV = (PFN_vkCmdTraceRaysNV)vkGetDeviceProcAddr( gRenderer.device, "vkCmdTraceRaysNV" );
	CreateRayTracingPipelinesNV = (PFN_vkCreateRayTracingPipelinesNV)vkGetDeviceProcAddr( gRenderer.device, "vkCreateRayTracingPipelinesNV" );
	CmdBuildAccelerationStructureNV = (PFN_vkCmdBuildAccelerationStructureNV)vkGetDeviceProcAddr( gRenderer.device, "vkCmdBuildAccelerationStructureNV" );
	CreateAccelerationStructureNV = (PFN_vkCreateAccelerationStructureNV)vkGetDeviceProcAddr( gRenderer.device, "vkCreateAccelerationStructureNV" );
	cassert( CmdTraceRaysNV != nullptr );
	cassert( CreateRayTracingPipelinesNV != nullptr );
	cassert( CmdBuildAccelerationStructureNV != nullptr );
	cassert( CreateAccelerationStructureNV != nullptr );
}

void CreateCommandBuffers()
{
    VkCommandPoolCreateInfo cmdPoolInfo = {};
    cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    cmdPoolInfo.queueFamilyIndex = gRenderer.graphicsQueueNodeIndex;
    cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    VK_CHECK( vkCreateCommandPool( gRenderer.device, &cmdPoolInfo, nullptr, &gRenderer.cmdPool ) );

    VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
    commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocateInfo.commandPool = gRenderer.cmdPool;
    commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferAllocateInfo.commandBufferCount = 4;

    VkCommandBuffer drawCmdBuffers[ 4 ];
    VK_CHECK( vkAllocateCommandBuffers( gRenderer.device, &commandBufferAllocateInfo, drawCmdBuffers ) );

    for (uint32_t i = 0; i < 4; ++i)
    {
        gRenderer.swapchainResources[ i ].drawCommandBuffer = drawCmdBuffers[ i ];
        const char* name = "drawCommandBuffer 0";
        if (i == 1)
        {
            name = "drawCommandBuffer 1";
        }
        else if (i == 2)
        {
            name = "drawCommandBuffer 2";
        }
        else if (i == 3)
        {
            name = "drawCommandBuffer 3";
        }
        
        SetObjectName( gRenderer.device, (uint64_t)drawCmdBuffers[ i ], VK_OBJECT_TYPE_COMMAND_BUFFER, name );
            
        VkSemaphoreCreateInfo semaphoreCreateInfo = {};
        semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VK_CHECK( vkCreateSemaphore( gRenderer.device, &semaphoreCreateInfo, nullptr, &gRenderer.swapchainResources[ i ].imageAcquiredSemaphore ) );
        SetObjectName( gRenderer.device, (uint64_t)gRenderer.swapchainResources[ i ].imageAcquiredSemaphore, VK_OBJECT_TYPE_SEMAPHORE, "imageAcquiredSemaphore" );

        VK_CHECK( vkCreateSemaphore( gRenderer.device, &semaphoreCreateInfo, nullptr, &gRenderer.swapchainResources[ i ].renderCompleteSemaphore ) );
        SetObjectName( gRenderer.device, (uint64_t)gRenderer.swapchainResources[ i ].imageAcquiredSemaphore, VK_OBJECT_TYPE_SEMAPHORE, "renderCompleteSemaphore" );

        VkFenceCreateInfo fenceCreateInfo = { VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, nullptr, VK_FENCE_CREATE_SIGNALED_BIT };

        VK_CHECK( vkCreateFence( gRenderer.device, &fenceCreateInfo, nullptr, &gRenderer.swapchainResources[ i ].fence ) );
    }
}

void CreateDevice()
{
    uint32_t gpuCount;
    VK_CHECK( vkEnumeratePhysicalDevices( gRenderer.instance, &gpuCount, nullptr ) );
    VK_CHECK( vkEnumeratePhysicalDevices( gRenderer.instance, &gpuCount, &gRenderer.physicalDevice ) );

    vkGetPhysicalDeviceProperties( gRenderer.physicalDevice, &gRenderer.properties );
    uint32_t queueCount;
    vkGetPhysicalDeviceQueueFamilyProperties( gRenderer.physicalDevice, &queueCount, nullptr );
    cassert( queueCount < 20 && "More queues than the array has elements!" );

    VkQueueFamilyProperties queueProps[ 20 ];
    vkGetPhysicalDeviceQueueFamilyProperties( gRenderer.physicalDevice, &queueCount, queueProps );

    uint32_t graphicsQueueIndex = 0;

    for (graphicsQueueIndex = 0; graphicsQueueIndex < queueCount; ++graphicsQueueIndex)
    {
        if (queueProps[ graphicsQueueIndex ].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            break;
        }
    }

    float queuePriorities = 0;
    VkDeviceQueueCreateInfo queueCreateInfo = {};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = graphicsQueueIndex;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queuePriorities;

    const char* enabledExtensions[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_NV_RAY_TRACING_EXTENSION_NAME };

    vkGetPhysicalDeviceFeatures( gRenderer.physicalDevice, &gRenderer.features );

    VkPhysicalDeviceFeatures enabledFeatures = {};
    enabledFeatures.shaderClipDistance = gRenderer.features.shaderClipDistance;
    enabledFeatures.shaderCullDistance = gRenderer.features.shaderCullDistance;
    enabledFeatures.textureCompressionBC = gRenderer.features.textureCompressionBC;
    enabledFeatures.fillModeNonSolid = gRenderer.features.fillModeNonSolid;
    enabledFeatures.samplerAnisotropy = gRenderer.features.samplerAnisotropy;
    enabledFeatures.fragmentStoresAndAtomics = gRenderer.features.fragmentStoresAndAtomics;
    enabledFeatures.shaderStorageImageExtendedFormats = gRenderer.features.shaderStorageImageExtendedFormats;
    enabledFeatures.vertexPipelineStoresAndAtomics = gRenderer.features.vertexPipelineStoresAndAtomics;

    VkDeviceCreateInfo deviceCreateInfo = {};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.queueCreateInfoCount = 1;
    deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
    deviceCreateInfo.pEnabledFeatures = &enabledFeatures;
    deviceCreateInfo.enabledExtensionCount = 2;
    deviceCreateInfo.ppEnabledExtensionNames = enabledExtensions;

    VK_CHECK( vkCreateDevice( gRenderer.physicalDevice, &deviceCreateInfo, nullptr, &gRenderer.device ) );

    vkGetPhysicalDeviceMemoryProperties( gRenderer.physicalDevice, &gRenderer.deviceMemoryProperties );
    vkGetDeviceQueue( gRenderer.device, graphicsQueueIndex, 0, &gRenderer.graphicsQueue );

    VkPhysicalDeviceRayTracingPropertiesNV rtProps{};
    rtProps.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PROPERTIES_NV;
    rtProps.maxRecursionDepth = 0;

    VkPhysicalDeviceProperties2 devProps;
    devProps.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
    devProps.pNext = &rtProps;
    devProps.properties = {};

    vkGetPhysicalDeviceProperties2( gRenderer.physicalDevice, &devProps );

    constexpr unsigned TextureCount = 1;
    VkDescriptorSetLayoutBinding layoutBindingImage = {};
    layoutBindingImage.binding = 0;
    layoutBindingImage.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    layoutBindingImage.descriptorCount = TextureCount;
    layoutBindingImage.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_NV;

    constexpr unsigned bindingCount = 1;
    VkDescriptorSetLayoutBinding bindings[ bindingCount ] = { layoutBindingImage };
        
    VkDescriptorSetLayoutCreateInfo setCreateInfo = {};
    setCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    setCreateInfo.bindingCount = bindingCount;
    setCreateInfo.pBindings = bindings;

    VK_CHECK( vkCreateDescriptorSetLayout( gRenderer.device, &setCreateInfo, nullptr, &gRenderer.descriptorSetLayout ) );
}

#ifdef _MSC_VER
void CreateSwapchain( unsigned& width, unsigned& height, int presentInterval, HWND hwnd )

#else
void CreateSwapchain( unsigned& width, unsigned& height, int presentInterval, struct xcb_connection_t* connection, unsigned window )
#endif
{
#if VK_USE_PLATFORM_WIN32_KHR
    VkWin32SurfaceCreateInfoKHR surfaceCreateInfo = {};
    surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    surfaceCreateInfo.hinstance = GetModuleHandle( nullptr );
    surfaceCreateInfo.hwnd = hwnd;
    VkResult err = vkCreateWin32SurfaceKHR( gRenderer.instance, &surfaceCreateInfo, nullptr, &gRenderer.surface );
    cassert( err == VK_SUCCESS );
#elif VK_USE_PLATFORM_XCB_KHR
    VkXcbSurfaceCreateInfoKHR surfaceCreateInfo = {};
    surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
    surfaceCreateInfo.connection = connection;
    surfaceCreateInfo.window = window;
    VkResult err = vkCreateXcbSurfaceKHR( gRenderer.instance, &surfaceCreateInfo, nullptr, &gRenderer.surface );
    cassert( err == VK_SUCCESS );
#else
#error "Unhandled platform!"
#endif

    uint32_t queueCount;
    vkGetPhysicalDeviceQueueFamilyProperties( gRenderer.physicalDevice, &queueCount, nullptr );
    cassert( queueCount > 0 && queueCount < 5 && "None or more queues than buffers have elements! Increase element count." );

    VkQueueFamilyProperties queueProps[ 5 ];
    vkGetPhysicalDeviceQueueFamilyProperties( gRenderer.physicalDevice, &queueCount, &queueProps[ 0 ] );

    VkBool32 supportsPresent[ 5 ];

    for (uint32_t i = 0; i < queueCount; ++i)
    {
        getPhysicalDeviceSurfaceSupportKHR( gRenderer.physicalDevice, i, gRenderer.surface, &supportsPresent[ i ] );
    }

    gRenderer.graphicsQueueNodeIndex = UINT32_MAX;
    uint32_t presentQueueNodeIndex = UINT32_MAX;

    for (uint32_t i = 0; i < queueCount; ++i)
    {
        if ((queueProps[ i ].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0)
        {
            if (gRenderer.graphicsQueueNodeIndex == UINT32_MAX)
            {
                gRenderer.graphicsQueueNodeIndex = i;
            }

            if (supportsPresent[ i ] == VK_TRUE)
            {
                gRenderer.graphicsQueueNodeIndex = i;
                presentQueueNodeIndex = i;
                break;
            }
        }
    }

    for (uint32_t q = 0; q < queueCount && presentQueueNodeIndex == UINT32_MAX; ++q)
    {
        if (supportsPresent[ q ] == VK_TRUE)
        {
            presentQueueNodeIndex = q;
        }
    }

    if (gRenderer.graphicsQueueNodeIndex == UINT32_MAX || presentQueueNodeIndex == UINT32_MAX)
    {
        printf( "graphics or present queue not found!\n" );
        return;
    }

    if (gRenderer.graphicsQueueNodeIndex != presentQueueNodeIndex)
    {
        printf( "graphics and present queues must have the same index!\n" );
        return;
    }

    uint32_t formatCount = 0;
    err = getPhysicalDeviceSurfaceFormatsKHR( gRenderer.physicalDevice, gRenderer.surface, &formatCount, nullptr );
    cassert( err == VK_SUCCESS && formatCount > 0 && formatCount < 20 && "Invalid format count" );
        
    VkSurfaceFormatKHR surfFormats[ 20 ];        
    err = getPhysicalDeviceSurfaceFormatsKHR( gRenderer.physicalDevice, gRenderer.surface, &formatCount, surfFormats );
    cassert( err == VK_SUCCESS && formatCount < 20 && "Too many formats!" );
    
    bool foundSRGB = false;

    for (uint32_t formatIndex = 0; formatIndex < formatCount; ++formatIndex)
    {
        if (surfFormats[ formatIndex ].format == VK_FORMAT_B8G8R8A8_SRGB || surfFormats[ formatIndex ].format == VK_FORMAT_R8G8B8A8_SRGB)
        {
            gRenderer.colorFormat = surfFormats[ formatIndex ].format;
            foundSRGB = true;
        }
    }

    if (!foundSRGB && formatCount == 1 && surfFormats[ 0 ].format == VK_FORMAT_UNDEFINED)
    {
        gRenderer.colorFormat = VK_FORMAT_B8G8R8A8_UNORM;
    }
    else if (!foundSRGB)
    {
        gRenderer.colorFormat = surfFormats[ 0 ].format;
    }

    VkSurfaceCapabilitiesKHR surfCaps;
    VK_CHECK( getPhysicalDeviceSurfaceCapabilitiesKHR( gRenderer.physicalDevice, gRenderer.surface, &surfCaps ) );

    uint32_t presentModeCount = 0;
    VK_CHECK( getPhysicalDeviceSurfacePresentModesKHR( gRenderer.physicalDevice, gRenderer.surface, &presentModeCount, nullptr ) );

    if (presentModeCount == 0 || presentModeCount > 10)
    {
        printf( "Invalid present mode count.\n" );
        return;
    }
    
    VkPresentModeKHR presentModes[ 10 ];
        
    VK_CHECK( getPhysicalDeviceSurfacePresentModesKHR( gRenderer.physicalDevice, gRenderer.surface, &presentModeCount, presentModes ) );

    VkExtent2D swapchainExtent = (surfCaps.currentExtent.width == (uint32_t)-1) ? VkExtent2D{ (uint32_t)width, (uint32_t)height } : surfCaps.currentExtent;

    if (surfCaps.currentExtent.width != (uint32_t)-1)
    {
        width = surfCaps.currentExtent.width;
        height = surfCaps.currentExtent.height;
    }

    gRenderer.width = width;
    gRenderer.height = height;
    
    const uint32_t desiredNumberOfSwapchainImages = ((surfCaps.maxImageCount > 0) && (surfCaps.minImageCount + 1 > surfCaps.maxImageCount)) ? surfCaps.maxImageCount : surfCaps.minImageCount + 1;
    
    VkSurfaceTransformFlagsKHR preTransform = (surfCaps.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) ? VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR : surfCaps.currentTransform;

    VkSwapchainCreateInfoKHR swapchainInfo = {};
    swapchainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainInfo.surface = gRenderer.surface;
    swapchainInfo.minImageCount = desiredNumberOfSwapchainImages;
    swapchainInfo.imageFormat = gRenderer.colorFormat;
    swapchainInfo.imageColorSpace = surfFormats[ 0 ].colorSpace;
    swapchainInfo.imageExtent = { swapchainExtent.width, swapchainExtent.height };
    swapchainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    swapchainInfo.preTransform = (VkSurfaceTransformFlagBitsKHR)preTransform;
    swapchainInfo.imageArrayLayers = 1;
    swapchainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchainInfo.queueFamilyIndexCount = 0;
    swapchainInfo.presentMode = presentInterval == 0 ? VK_PRESENT_MODE_IMMEDIATE_KHR : VK_PRESENT_MODE_FIFO_KHR;
    swapchainInfo.clipped = VK_TRUE;
    swapchainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

    VK_CHECK( createSwapchainKHR( gRenderer.device, &swapchainInfo, nullptr, &gRenderer.swapchain ) );
	SetObjectName( gRenderer.device, (uint64_t)gRenderer.swapchain, VK_OBJECT_TYPE_SWAPCHAIN_KHR, "swap chain" );

    VK_CHECK( getSwapchainImagesKHR( gRenderer.device, gRenderer.swapchain, &gRenderer.swapchainImageCount, nullptr ) );

    if (gRenderer.swapchainImageCount == 0 || gRenderer.swapchainImageCount > 4)
    {
        printf( "Invalid count of swapchain images!\n ");
        return;
    }

    VkImage images[ 4 ];
    VK_CHECK( getSwapchainImagesKHR( gRenderer.device, gRenderer.swapchain, &gRenderer.swapchainImageCount, images ) );

    VkCommandBufferBeginInfo cmdBufInfo = {};
    cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    VK_CHECK( vkBeginCommandBuffer( gRenderer.swapchainResources[ 0 ].drawCommandBuffer, &cmdBufInfo ) );

    for (uint32_t i = 0; i < gRenderer.swapchainImageCount; ++i)
    {
        VkImageViewCreateInfo colorAttachmentView = {};
        colorAttachmentView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        colorAttachmentView.format = gRenderer.colorFormat;
        colorAttachmentView.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
        colorAttachmentView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        colorAttachmentView.subresourceRange.levelCount = 1;
        colorAttachmentView.subresourceRange.layerCount = 1;
        colorAttachmentView.viewType = VK_IMAGE_VIEW_TYPE_2D;
        colorAttachmentView.image = images[ i ];
        VK_CHECK( vkCreateImageView( gRenderer.device, &colorAttachmentView, nullptr, &gRenderer.swapchainResources[ i ].view ) );
        SetObjectName( gRenderer.device, (uint64_t)gRenderer.swapchainResources[ i ].view, VK_OBJECT_TYPE_IMAGE_VIEW, "swapchain view" );

        gRenderer.swapchainResources[ i ].image = images[ i ];

        SetObjectName( gRenderer.device, (uint64_t)images[ i ], VK_OBJECT_TYPE_IMAGE, "swapchain image" );

        SetImageLayout( gRenderer.swapchainResources[ 0 ].drawCommandBuffer, gRenderer.swapchainResources[ i ].image,
                        VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, 1, 0, 1, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT );
    }

	VK_CHECK( vkEndCommandBuffer( gRenderer.swapchainResources[ 0 ].drawCommandBuffer ) );

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &gRenderer.swapchainResources[ 0 ].drawCommandBuffer;

	VK_CHECK( vkQueueSubmit( gRenderer.graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE ) );
	VK_CHECK( vkQueueWaitIdle( gRenderer.graphicsQueue ) );
}

void EndFrame()
{
    VK_CHECK( vkEndCommandBuffer( gRenderer.swapchainResources[ gRenderer.currentBuffer ].drawCommandBuffer ) );

    VkPipelineStageFlags pipelineStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pWaitDstStageMask = &pipelineStages;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &gRenderer.swapchainResources[ gRenderer.frameIndex ].imageAcquiredSemaphore;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &gRenderer.swapchainResources[ gRenderer.frameIndex ].renderCompleteSemaphore;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &gRenderer.swapchainResources[ gRenderer.currentBuffer ].drawCommandBuffer;

    VK_CHECK( vkQueueSubmit( gRenderer.graphicsQueue, 1, &submitInfo, gRenderer.swapchainResources[ gRenderer.frameIndex ].fence ) );

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &gRenderer.swapchain;
    presentInfo.pImageIndices = &gRenderer.currentBuffer;
    presentInfo.pWaitSemaphores = &gRenderer.swapchainResources[ gRenderer.frameIndex ].renderCompleteSemaphore;
    presentInfo.waitSemaphoreCount = 1;
    VkResult err = queuePresentKHR( gRenderer.graphicsQueue, &presentInfo );

    if (err == VK_ERROR_OUT_OF_DATE_KHR)
    {
        printf( "Swapchain is out of date!\n" );
        // Handle resizing etc.
    }
    else if (err == VK_SUBOPTIMAL_KHR)
    {
        printf( "Swapchain is suboptimal!\n" );
    }
    else
    {
        cassert( err == VK_SUCCESS );
    }

    gRenderer.frameIndex = (gRenderer.frameIndex + 1) % gRenderer.swapchainImageCount;
}

struct File
{
    unsigned char* data = nullptr;
    unsigned size = 0;
};

File LoadFile( const char* path )
{
    File outFile;
    FILE* file = fopen( path, "rb" );

    if (file)
    {
        fseek( file, 0, SEEK_END );
        auto length = ftell( file );
        fseek( file, 0, SEEK_SET );
        outFile.data = new unsigned char[ length ];
        outFile.size = (unsigned)length;
        fread( outFile.data, 1, length, file );
        fclose( file );
    }
    else
    {
        printf( "Could not open file %s\n", path );
    }

    return outFile;
}

void LoadShaders()
{
    {
        File rayHitFile = LoadFile( "rahit.spv" );
    
        VkShaderModuleCreateInfo moduleCreateInfo{};
        moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        moduleCreateInfo.codeSize = rayHitFile.size;
        moduleCreateInfo.pCode = (const uint32_t*)rayHitFile.data;

        VkResult err = vkCreateShaderModule( gRenderer.device, &moduleCreateInfo, nullptr, &gRenderer.rayHitModule );
        cassert( err == VK_SUCCESS );
        SetObjectName( gRenderer.device, (uint64_t)gRenderer.rayHitModule, VK_OBJECT_TYPE_SHADER_MODULE, "rayHit" );
    }
    {
        File rayMissFile = LoadFile( "ramiss.spv" );
    
        VkShaderModuleCreateInfo moduleCreateInfo{};
        moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        moduleCreateInfo.codeSize = rayMissFile.size;
        moduleCreateInfo.pCode = (const uint32_t*)rayMissFile.data;

        VkResult err = vkCreateShaderModule( gRenderer.device, &moduleCreateInfo, nullptr, &gRenderer.rayMissModule );
        cassert( err == VK_SUCCESS );
        SetObjectName( gRenderer.device, (uint64_t)gRenderer.rayMissModule, VK_OBJECT_TYPE_SHADER_MODULE, "rayMiss" );
    }
	{
		File rayGenFile = LoadFile( "raygen.spv" );

		VkShaderModuleCreateInfo moduleCreateInfo{};
		moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		moduleCreateInfo.codeSize = rayGenFile.size;
		moduleCreateInfo.pCode = (const uint32_t*)rayGenFile.data;

		VkResult err = vkCreateShaderModule( gRenderer.device, &moduleCreateInfo, nullptr, &gRenderer.rayGenModule );
		cassert( err == VK_SUCCESS );
		SetObjectName( gRenderer.device, (uint64_t)gRenderer.rayGenModule, VK_OBJECT_TYPE_SHADER_MODULE, "rayGen" );
	}
}

void CreatePSO()
{
	VkPipelineLayoutCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	createInfo.setLayoutCount = 1;
	createInfo.pSetLayouts = &gRenderer.descriptorSetLayout;

	VkPushConstantRange pushConstantRange = {};
	pushConstantRange.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_NV;
	pushConstantRange.size = sizeof( int );

	createInfo.pushConstantRangeCount = 1;
	createInfo.pPushConstantRanges = &pushConstantRange;
	VK_CHECK( vkCreatePipelineLayout( gRenderer.device, &createInfo, nullptr, &gRenderer.pipelineLayout ) );

    constexpr unsigned StageCount = 1;
	VkPipelineShaderStageCreateInfo stages[ StageCount ] = {};

	stages[ 0 ].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	stages[ 0 ].stage = VK_SHADER_STAGE_RAYGEN_BIT_NV;
	stages[ 0 ].module = gRenderer.rayGenModule;
	stages[ 0 ].pName = "main";

	/*stages[ 0 ].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stages[ 0 ].stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV;
    stages[ 0 ].module = gRenderer.rayHitModule;
    stages[ 0 ].pName = "main";

	stages[ 2 ].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	stages[ 2 ].stage = VK_SHADER_STAGE_MISS_BIT_NV;
	stages[ 2 ].module = gRenderer.rayMissModule;
	stages[ 2 ].pName = "main";*/

	gRenderer.sbt.groups[ 0 ].sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_NV;
	gRenderer.sbt.groups[ 0 ].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_NV;
	gRenderer.sbt.groups[ 0 ].anyHitShader = VK_SHADER_UNUSED_NV;
	gRenderer.sbt.groups[ 0 ].closestHitShader = VK_SHADER_UNUSED_NV;
	gRenderer.sbt.groups[ 0 ].generalShader = 0;
	gRenderer.sbt.groups[ 0 ].intersectionShader = VK_SHADER_UNUSED_NV;

	VkRayTracingPipelineCreateInfoNV info{};
	info.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_NV;
	info.pNext = nullptr;
	info.flags = 0;
	info.stageCount = StageCount;
	info.pStages = stages;
	info.maxRecursionDepth = 1;
	info.layout = gRenderer.pipelineLayout;
	info.basePipelineHandle = VK_NULL_HANDLE;
	info.basePipelineIndex = 0;
	info.groupCount = gRenderer.sbt.groupCount;
	info.pGroups = gRenderer.sbt.groups;

	VK_CHECK( CreateRayTracingPipelinesNV( gRenderer.device, nullptr, 1, &info, nullptr, &gRenderer.psoRayGen ) );
}

static void CreateDescriptorSet()
{
	VkDescriptorPoolSize poolSize = {};
	poolSize.descriptorCount = 1;
	poolSize.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;

	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = 1;
	poolInfo.pPoolSizes = &poolSize;
	poolInfo.maxSets = 1;
	poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

	VK_CHECK( vkCreateDescriptorPool( gRenderer.device, &poolInfo, nullptr, &gRenderer.descriptorPool ) );

	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = gRenderer.descriptorPool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &gRenderer.descriptorSetLayout;

	VK_CHECK( vkAllocateDescriptorSets( gRenderer.device, &allocInfo, &gRenderer.descriptorSet ) );

	VkDescriptorImageInfo samplerDesc = {};
	samplerDesc.sampler = gRenderer.sampler;
	samplerDesc.imageView = gRenderer.outputImageView;
	samplerDesc.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

	VkWriteDescriptorSet imageSet = {};
	imageSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	imageSet.dstSet = gRenderer.descriptorSet;
	imageSet.descriptorCount = 1;
	imageSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	imageSet.pImageInfo = &samplerDesc;
	imageSet.dstBinding = 0;

	vkUpdateDescriptorSets( gRenderer.device, 1, &imageSet, 0, nullptr );
}

static void CreateOutputImage()
{
	VkImageCreateInfo imageInfo = {};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.format = VK_FORMAT_B8G8R8A8_UNORM;
	imageInfo.extent.width = gRenderer.width;
	imageInfo.extent.height = gRenderer.height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	VK_CHECK( vkCreateImage( gRenderer.device, &imageInfo, nullptr, &gRenderer.outputImage ) );
	SetObjectName( gRenderer.device, (uint64_t)gRenderer.outputImage, VK_OBJECT_TYPE_IMAGE, "output image" );

	VkMemoryRequirements memReqs = {};
	vkGetImageMemoryRequirements( gRenderer.device, gRenderer.outputImage, &memReqs );

	VkMemoryAllocateInfo memAllocInfo = {};
	memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memAllocInfo.allocationSize = memReqs.size;
	memAllocInfo.memoryTypeIndex = GetMemoryType( memReqs.memoryTypeBits, gRenderer.deviceMemoryProperties, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT );

	VK_CHECK( vkAllocateMemory( gRenderer.device, &memAllocInfo, nullptr, &gRenderer.outputImageMemory ) );
	SetObjectName( gRenderer.device, (uint64_t)gRenderer.outputImageMemory, VK_OBJECT_TYPE_DEVICE_MEMORY, "output image memory" );

	VK_CHECK( vkBindImageMemory( gRenderer.device, gRenderer.outputImage, gRenderer.outputImageMemory, 0 ) );

	VkImageViewCreateInfo viewInfo = {};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = imageInfo.format;
	viewInfo.components = { VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY };
	viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.image = gRenderer.outputImage;
	VK_CHECK( vkCreateImageView( gRenderer.device, &viewInfo, nullptr, &gRenderer.outputImageView ) );

	VkSamplerCreateInfo samplerInfo = {};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_NEAREST;
	samplerInfo.minFilter = VK_FILTER_NEAREST;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = samplerInfo.addressModeU;
	samplerInfo.addressModeW = samplerInfo.addressModeU;
	samplerInfo.mipLodBias = 0;
	samplerInfo.compareOp = VK_COMPARE_OP_NEVER;
	samplerInfo.minLod = 0;
	samplerInfo.maxLod = 1;
	samplerInfo.maxAnisotropy = 1;
	samplerInfo.anisotropyEnable = VK_FALSE;
	samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
	VK_CHECK( vkCreateSampler( gRenderer.device, &samplerInfo, nullptr, &gRenderer.sampler ) );

	VkCommandBufferBeginInfo cmdBufInfo = {};
	cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cmdBufInfo.pNext = nullptr;

	VK_CHECK( vkBeginCommandBuffer( gRenderer.swapchainResources[ gRenderer.currentBuffer ].drawCommandBuffer, &cmdBufInfo ) );

	SetImageLayout( gRenderer.swapchainResources[ gRenderer.currentBuffer ].drawCommandBuffer, gRenderer.outputImage, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, 1, 0, 1, VK_PIPELINE_STAGE_TRANSFER_BIT );

	VK_CHECK( vkEndCommandBuffer( gRenderer.swapchainResources[ 0 ].drawCommandBuffer ) );

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &gRenderer.swapchainResources[ 0 ].drawCommandBuffer;

	VK_CHECK( vkQueueSubmit( gRenderer.graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE ) );
	VK_CHECK( vkQueueWaitIdle( gRenderer.graphicsQueue ) );
}

void CreateBuffer( VkBuffer& outBuffer, VkDeviceMemory& outMemory, unsigned size, const char* debugName )
{
    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    VK_CHECK( vkCreateBuffer( gRenderer.device, &bufferInfo, nullptr, &outBuffer ) );
    SetObjectName( gRenderer.device, (uint64_t)outBuffer, VK_OBJECT_TYPE_BUFFER, debugName );

    VkMemoryRequirements memReqs;
    vkGetBufferMemoryRequirements( gRenderer.device, outBuffer, &memReqs );
    
    VkMemoryAllocateInfo memAlloc = {};
    memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memAlloc.allocationSize = memReqs.size;
    memAlloc.memoryTypeIndex = GetMemoryType( memReqs.memoryTypeBits, gRenderer.deviceMemoryProperties, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT );
    VK_CHECK( vkAllocateMemory( gRenderer.device, &memAlloc, nullptr, &outMemory ) );
    SetObjectName( gRenderer.device, (uint64_t)outMemory, VK_OBJECT_TYPE_DEVICE_MEMORY, debugName );

    VK_CHECK( vkBindBufferMemory( gRenderer.device, outBuffer, outMemory, 0 ) );

}
void CreateRaytracerResources()
{
    unsigned numberOfGroups = 666; // FIXME: Fill this
    unsigned sizeOfGroup = 32; // FIXME: Fill this
    
    CreateBuffer( gRenderer.raytracer.rayGenBindingTable, gRenderer.raytracer.rayGenBindingMemory, numberOfGroups * sizeOfGroup, "rayGenBindingTable" );
    CreateBuffer( gRenderer.raytracer.rayMissBindingTable, gRenderer.raytracer.rayMissBindingMemory, numberOfGroups * sizeOfGroup, "rayMissBindingTable" );
    CreateBuffer( gRenderer.raytracer.rayHitBindingTable, gRenderer.raytracer.rayHitBindingMemory, numberOfGroups * sizeOfGroup, "rayHitBindingTable" );
    CreateBuffer( gRenderer.raytracer.rayCallableBindingTable, gRenderer.raytracer.rayCallableBindingMemory, numberOfGroups * sizeOfGroup, "rayCallableBindingTable" );
}

#ifdef _MSC_VER
void aeCreateRenderer( unsigned& width, unsigned& height, HWND hwnd )
#else
void aeCreateRenderer( unsigned& width, unsigned& height, xcb_connection_t* connection, unsigned win )
#endif
{
    CreateInstance();
    CreateDevice();
    LoadFunctionPointers();
    CreateCommandBuffers();
#ifdef _MSC_VER
    CreateSwapchain( width, height, 1, hwnd );
#else
    CreateSwapchain( width, height, 1, connection, win );
#endif
    LoadShaders();
	CreateRaytracerResources();
	CreatePSO();
	CreateOutputImage();
	CreateDescriptorSet();
}

void aeBeginFrame()
{
    vkWaitForFences( gRenderer.device, 1, &gRenderer.swapchainResources[ gRenderer.frameIndex ].fence, VK_TRUE, UINT64_MAX );
    vkResetFences( gRenderer.device, 1, &gRenderer.swapchainResources[ gRenderer.frameIndex ].fence );

    VkResult err = VK_SUCCESS;
    
    do
    {
        err = acquireNextImageKHR( gRenderer.device, gRenderer.swapchain, UINT64_MAX, gRenderer.swapchainResources[ gRenderer.frameIndex ].imageAcquiredSemaphore, VK_NULL_HANDLE, &gRenderer.currentBuffer );
        
        if (err == VK_ERROR_OUT_OF_DATE_KHR)
        {
            printf( "Swapchain is out of date!\n" );
            break;
            // Handle resizing etc.
        }
        else if (err == VK_SUBOPTIMAL_KHR)
        {
            printf( "Swapchain is suboptimal!\n" );
            break;
        }
        else
        {
            cassert( err == VK_SUCCESS );
        }
    } while( err != VK_SUCCESS );

    VkCommandBufferBeginInfo cmdBufInfo = {};
    cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cmdBufInfo.pNext = nullptr;

    VK_CHECK( vkBeginCommandBuffer( gRenderer.swapchainResources[ gRenderer.currentBuffer ].drawCommandBuffer, &cmdBufInfo ) );

    const VkViewport viewport = { 0, 0, (float)gRenderer.width, (float)gRenderer.height, 0.0f, 1.0f };
    vkCmdSetViewport( gRenderer.swapchainResources[ gRenderer.currentBuffer ].drawCommandBuffer, 0, 1, &viewport );

    const VkRect2D scissor = { { 0, 0 }, { (uint32_t)gRenderer.width, (uint32_t)gRenderer.height } };
    vkCmdSetScissor( gRenderer.swapchainResources[ gRenderer.currentBuffer ].drawCommandBuffer, 0, 1, &scissor );
}

void TraceRays()
{
	vkCmdBindPipeline( gRenderer.swapchainResources[ gRenderer.currentBuffer ].drawCommandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_NV, gRenderer.psoRayGen );

	CmdTraceRaysNV(
		gRenderer.swapchainResources[ gRenderer.currentBuffer ].drawCommandBuffer,
		gRenderer.raytracer.rayGenBindingTable,
		0,
		gRenderer.raytracer.rayMissBindingTable,
		0,
		0,
		gRenderer.raytracer.rayHitBindingTable,
		0,
		0,
		gRenderer.raytracer.rayCallableBindingTable,
		0,
		0,
		gRenderer.width,
		gRenderer.height,
		1
	);

	SetImageLayout( gRenderer.swapchainResources[ gRenderer.currentBuffer ].drawCommandBuffer, gRenderer.swapchainResources[ gRenderer.currentBuffer].image, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, 0, 1, VK_PIPELINE_STAGE_TRANSFER_BIT );

	VkImageCopy copy_region = {};
	copy_region.srcSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
	copy_region.srcOffset = { 0, 0, 0 };
	copy_region.dstSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
	copy_region.dstOffset = { 0, 0, 0 };
	copy_region.extent = { (uint32_t)gRenderer.width, (uint32_t)gRenderer.height, 1 };
	vkCmdCopyImage( gRenderer.swapchainResources[ gRenderer.currentBuffer ].drawCommandBuffer, gRenderer.outputImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		gRenderer.swapchainResources[ gRenderer.currentBuffer ].image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_region );

	SetImageLayout( gRenderer.swapchainResources[ gRenderer.currentBuffer ].drawCommandBuffer, gRenderer.swapchainResources[ gRenderer.currentBuffer ].image, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, 1, 0, 1, VK_PIPELINE_STAGE_TRANSFER_BIT );
}

void aeEndFrame()
{
    VK_CHECK( vkEndCommandBuffer( gRenderer.swapchainResources[ gRenderer.currentBuffer ].drawCommandBuffer ) );

    VkPipelineStageFlags pipelineStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pWaitDstStageMask = &pipelineStages;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &gRenderer.swapchainResources[ gRenderer.frameIndex ].imageAcquiredSemaphore;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &gRenderer.swapchainResources[ gRenderer.frameIndex ].renderCompleteSemaphore;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &gRenderer.swapchainResources[ gRenderer.currentBuffer ].drawCommandBuffer;

    VK_CHECK( vkQueueSubmit( gRenderer.graphicsQueue, 1, &submitInfo, gRenderer.swapchainResources[ gRenderer.frameIndex ].fence ) );

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &gRenderer.swapchain;
    presentInfo.pImageIndices = &gRenderer.currentBuffer;
    presentInfo.pWaitSemaphores = &gRenderer.swapchainResources[ gRenderer.frameIndex ].renderCompleteSemaphore;
    presentInfo.waitSemaphoreCount = 1;
    VkResult err = queuePresentKHR( gRenderer.graphicsQueue, &presentInfo );

    if (err == VK_ERROR_OUT_OF_DATE_KHR)
    {
        printf( "Swapchain is out of date!\n" );
        // Handle resizing etc.
    }
    else if (err == VK_SUBOPTIMAL_KHR)
    {
        printf( "Swapchain is suboptimal!\n" );
    }
    else
    {
        cassert( err == VK_SUCCESS );
    }

    gRenderer.frameIndex = (gRenderer.frameIndex + 1) % gRenderer.swapchainImageCount;
}
