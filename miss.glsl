#version 460
#extension GL_NV_ray_tracing : require

layout(location = 0) rayPayloadInNV vec3 outColor;
//layout(location = 1) hitAttributeNV vec2 hitAttribs;

void main()
{
    const vec3 barycentrics = vec3( 0, 0, 0 );//)vec3( 1.0f - hitAttribs.x - hitAttribs.y, hitAttribs.x, hitAttribs.y );
    outColor = vec3( barycentrics );
}
