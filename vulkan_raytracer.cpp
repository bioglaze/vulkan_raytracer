// vulkan_raytracer
//
// Author: Timo Wiren
// License: MIT
// Modified: 2020-01-16
#include <stdio.h>

#ifdef _MSC_VER
#define cassert(c) { if (!(c)) { *(int* )0 = 0;} }
#else
#define cassert(c) { if (!(c)) { __builtin_trap(); } }
#endif

#define VK_CHECK( x ) { VkResult res = (x); cassert( res == VK_SUCCESS ); }

int cstrcmp( const char* str1, const char* str2 )
{
    while (*str1 && (*str1 == *str2))
    {
        ++str1;
        ++str2;
    }

    return *(const unsigned char*)str1 - *(const unsigned char*)str2;
}

#ifdef _MSC_VER
#include "window_win32.cpp"
#else
#include "window_xcb.cpp"
#endif
#include "renderer_vulkan.cpp"

int main()
{
    unsigned width = 1280;
    unsigned height = 720;
    aeWindow window = aeCreateWindow( width, height, "Vulkan Raytracer" );
#ifdef _MSC_VER
    aeCreateRenderer( width, height, window.hwnd );
#else
    aeCreateRenderer( width, height, window.connection, window.window );
#endif

    //aeShader shader;
    
    bool shouldQuit = false;

    while (!shouldQuit)
    {
        aePumpWindowEvents( window );

        bool eventsHandled = false;

        while (!eventsHandled)
        {
            const aeWindowEvent& event = aePopWindowEvent( window );

            if (event.type == aeWindowEvent::Type::Empty)
            {
                eventsHandled = true;
            }
            else if (event.type == aeWindowEvent::Type::KeyDown || event.type == aeWindowEvent::Type::Close)
            {
                shouldQuit = true;
                eventsHandled = true;
            }
        }

        aeBeginFrame();
        aeBeginRenderPass();
        aeEndRenderPass();
        aeEndFrame();
    }
}
