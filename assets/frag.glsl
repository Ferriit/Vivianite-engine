#version 460 core

in vec3 vertexColor;
in vec3 normalColor;

out vec4 FragColor;

void main() {
    FragColor = vec4(vertexColor, 1.0);
    FragColor = vec4(normalColor, 1.0);
}
