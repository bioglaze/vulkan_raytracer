#include <vulkan/vulkan.h>

struct SwapchainResource
{
    VkImage image = VK_NULL_HANDLE;
    VkImageView view = VK_NULL_HANDLE;
    VkCommandBuffer drawCommandBuffer = VK_NULL_HANDLE;
    VkSemaphore renderCompleteSemaphore = VK_NULL_HANDLE;
    VkSemaphore imageAcquiredSemaphore = VK_NULL_HANDLE;
    VkFence fence = VK_NULL_HANDLE;
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
    VkFormat depthFormat = VK_FORMAT_UNDEFINED;
    VkPhysicalDeviceMemoryProperties deviceMemoryProperties;
    VkDebugUtilsMessengerEXT dbgMessenger = VK_NULL_HANDLE;
    SwapchainResource swapchainResources[ 3 ] = {};
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    VkSwapchainKHR swapchain = VK_NULL_HANDLE;
    uint32_t graphicsQueueNodeIndex = 0;
    VkCommandPool cmdPool;
    uint32_t currentBuffer = 0;
    unsigned frameIndex = 0;
};

Renderer gRenderer;

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
    
    //assert( !"Vulkan debug message" );

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
    appInfo.pApplicationName = "ExperimentalEngine";
    appInfo.pEngineName = "ExperimentalEngine";
    appInfo.apiVersion = VK_API_VERSION_1_1;

    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties( nullptr, &extensionCount, nullptr );
    VkExtensionProperties extensions[ 20 ];
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
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME
    };

    VkInstanceCreateInfo instanceCreateInfo = {};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pApplicationInfo = &appInfo;
    instanceCreateInfo.enabledExtensionCount = hasDebugUtils ? 3 : 2;
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
    commandBufferAllocateInfo.commandBufferCount = 3;

    VkCommandBuffer drawCmdBuffers[ 3 ];
    VK_CHECK( vkAllocateCommandBuffers( gRenderer.device, &commandBufferAllocateInfo, drawCmdBuffers ) );

    for (uint32_t i = 0; i < 3; ++i)
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
    deviceCreateInfo.enabledExtensionCount = 1;
    deviceCreateInfo.ppEnabledExtensionNames = enabledExtensions;

    VK_CHECK( vkCreateDevice( gRenderer.physicalDevice, &deviceCreateInfo, nullptr, &gRenderer.device ) );

    vkGetPhysicalDeviceMemoryProperties( gRenderer.physicalDevice, &gRenderer.deviceMemoryProperties );
    vkGetDeviceQueue( gRenderer.device, graphicsQueueIndex, 0, &gRenderer.graphicsQueue );

    const VkFormat depthFormats[ 4 ] = { VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D16_UNORM_S8_UINT, VK_FORMAT_D16_UNORM };
        
    for (unsigned i = 0; i < 4; ++i)
    {
        VkFormatProperties formatProps;
        vkGetPhysicalDeviceFormatProperties( gRenderer.physicalDevice, depthFormats[ i ], &formatProps );

        if ((formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) && gRenderer.depthFormat == VK_FORMAT_UNDEFINED)
        {
            gRenderer.depthFormat = depthFormats[ i ];
        }
    }

    cassert( gRenderer.depthFormat != VK_FORMAT_UNDEFINED && "undefined depth format!" );

    VkPhysicalDeviceRayTracingPropertiesNV rtProps{};
    rtProps.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PROPERTIES_NV;
    rtProps.maxRecursionDepth = 0;

    VkPhysicalDeviceProperties2 devProps;
    devProps.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
    devProps.pNext = &rtProps;
    devProps.properties = {};

    vkGetPhysicalDeviceProperties2( gRenderer.physicalDevice, &devProps );
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

    const uint32_t desiredNumberOfSwapchainImages = ((surfCaps.maxImageCount > 0) && (surfCaps.minImageCount + 1 > surfCaps.maxImageCount)) ? surfCaps.maxImageCount : surfCaps.minImageCount + 1;
    
    VkSurfaceTransformFlagsKHR preTransform = (surfCaps.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) ? VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR : surfCaps.currentTransform;

    VkSwapchainCreateInfoKHR swapchainInfo = {};
    swapchainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainInfo.surface = gRenderer.surface;
    swapchainInfo.minImageCount = desiredNumberOfSwapchainImages;
    swapchainInfo.imageFormat = gRenderer.colorFormat;
    swapchainInfo.imageColorSpace = surfFormats[ 0 ].colorSpace;
    swapchainInfo.imageExtent = { swapchainExtent.width, swapchainExtent.height };
    swapchainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
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

    if (gRenderer.swapchainImageCount == 0 || gRenderer.swapchainImageCount > 3)
    {
        printf( "Invalid count of swapchain images!\n ");
        return;
    }

    VkImage images[ 3 ];
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
}