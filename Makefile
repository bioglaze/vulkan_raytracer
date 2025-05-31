LINKER := -ldl -lxcb -lxcb-ewmh -lxcb-keysyms -lxcb-icccm -lX11-xcb -lvulkan
FLAGS := -std=c++17 -Wall -Wextra -Wshadow -Wunreachable-code -Wno-unused-function -pedantic -msse3
DEFINES := -DVK_USE_PLATFORM_XCB_KHR -DSURFACE_EXTENSION_NAME=VK_KHR_XCB_SURFACE_EXTENSION_NAME -D_DEBUG
CC := clang++
SANITIZERS := -g -fsanitize=address,undefined

SHADER1 := "$(shell glslangValidator -V --target-env vulkan1.2 -S rgen gen.glsl -o raygen.spv)"
SHADER2 := "$(shell glslangValidator -V --target-env vulkan1.2 -S rchit hit.glsl -o rahit.spv)"
SHADER3 := "$(shell glslangValidator -V --target-env vulkan1.2 -S rmiss miss.glsl -o ramiss.spv)"

all:
	echo $(SHADER1)
	echo $(SHADER2)
	echo $(SHADER3)
	$(CC) $(FLAGS) $(DEFINES) $(SANITIZERS) vulkan_raytracer.cpp $(LINKER) -o vulkan_raytracer

