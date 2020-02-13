#version 460
#extension GL_NV_ray_tracing : require

layout(location = 0) rayPayloadInNV vec3 outColor;
//layout(location = 1) hitAttributeNV vec2 hitAttribs;

void main()
{
    outColor = vec3( 0.0f, 0.0f, 0.5f );
}
