#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoord;

uniform vec2 texScale; // scale first, then offset
uniform vec2 texOffset;
uniform float scale;
uniform vec2 offset;
uniform vec2 windowSize;

void main()
{
    vec2 pos = vec2((aPos.x*scale+offset.x - windowSize.x/2.0f) * 2.0f / windowSize.x,
                    (aPos.y*scale+offset.y - windowSize.y/2.0f) * -2.0f / windowSize.y);
    gl_Position = vec4(pos, aPos.z, 1);

    vec2 tc = vec2(aTexCoord.x * texScale.x + texOffset.x,
    	           -aTexCoord.y * texScale.y - texOffset.y - 8);
    tc = vec2(tc.x / 112.0f, tc.y / 64.0f);
    TexCoord = tc;
}