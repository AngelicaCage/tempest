#version 330 core
out vec4 FragColor;
  
in vec2 TexCoord;

uniform sampler2D texture1;
uniform vec4 textColor;

void main()
{
    vec2 tex_size = textureSize(texture1, 1);
    vec2 tc = vec2(TexCoord.x / tex_size.x, TexCoord.y / tex_size.y);
//    FragColor = texture(texture1, TexCoord);
    FragColor = vec4(textColor.rgb, texture(texture1, TexCoord).a * textColor.a);
}