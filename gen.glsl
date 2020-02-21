#version 460
#extension GL_NV_ray_tracing : require

layout(location = 0) rayPayloadNV vec3 outColor;
//layout(location = 1) hitAttributeNV vec2 hitAttribs;

void main()
{
    const vec2 uv = vec2( gl_LaunchIDNV.xy ) / vec2( gl_LaunchSizeNV.xy - 1 );

    const vec3 origin = vec3(uv.x, 1.0f - uv.y, -1.0f);
    const vec3 direction = vec3(0.0f, 0.0f, 1.0f);

    outColor = vec3( 1.0f, 0.0f, 0.0f );
}
