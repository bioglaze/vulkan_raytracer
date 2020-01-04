LINKER := -ldl -lxcb -lxcb-ewmh -lxcb-keysyms -lxcb-icccm -lX11-xcb -lvulkan
FLAGS := -std=c++17 -Wall -Wextra -Wshadow -Wunreachable-code -Wno-unused-function -pedantic -msse3 -DSURFACE_EXTENSION_NAME=VK_KHR_XCB_SURFACE_EXTENSION_NAME -D_DEBUG
DEFINES := -DVK_USE_PLATFORM_XCB_KHR
CC := clang++-8
SANITIZERS := -g -fsanitize=address,undefined

# SHADER1 := "$(shell glslangValidator -D -V -S vert -e mainVS water.hlsl -o ../build/shader_vs.spv)"

all:
#	echo $(SHADER1)
	$(CC) $(FLAGS) $(DEFINES) $(SANITIZERS) vulkan_raytracer.cpp $(LINKER) -o vulkan_raytracer

