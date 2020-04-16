#version 460
#extension GL_EXT_ray_tracing : require

//layout(location = 0) rayPayloadEXT vec3 outColor;
layout(binding=0, set=0, rgba8) uniform image2D image;
//layout(location = 1) hitAttributeEXT vec2 hitAttribs;

void main()
{
    const vec2 uv = vec2( gl_LaunchIDEXT.xy ) / vec2( gl_LaunchSizeEXT.xy - 1 );

    const vec3 origin = vec3(uv.x, 1.0f - uv.y, -1.0f);
    const vec3 direction = vec3(0.0f, 0.0f, 1.0f);

   // outColor = vec3( 1.0f, 0.0f, 0.0f );
   imageStore( image, ivec2( gl_LaunchIDEXT.xy ), vec4( 1.0, 0.0, 0.0, 0.0 ) );
}
