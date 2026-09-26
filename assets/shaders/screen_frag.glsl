#version 460 core

in vec2 texcoord;

uniform sampler2D screen_texture;

layout(location = 0) out vec4 color;

void main() {
    color = texture(screen_texture, texcoord);
}
