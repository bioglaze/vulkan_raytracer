glslangValidator -V --target-env vulkan1.2 -S rchit hit.glsl -o rahit.spv
glslangValidator -V --target-env vulkan1.2 -S rmiss miss.glsl -o ramiss.spv
glslangValidator -V --target-env vulkan1.2 -S rgen gen.glsl -o raygen.spv
pause


