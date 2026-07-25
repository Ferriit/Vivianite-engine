#version 460 core

in vec3 vertex_color;
in vec3 vertex_cormal;

out vec4 FragColor;

void main() {
    FragColor = vec4(vertex_ormal, 1.0);
}
