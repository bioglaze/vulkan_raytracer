#version 460
#extension GL_EXT_ray_tracing : require

layout(location = 0) rayPayloadInEXT vec3 outColor;
//layout(location = 1) hitAttributeEXT vec2 hitAttribs;

void main()
{
    outColor = vec3( 0.0f, 0.0f, 0.5f );
}
