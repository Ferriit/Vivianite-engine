#version 460 core

in vec3 vertexColor;
in vec3 vertexNormal;

out vec4 FragColor;

void main() {
    FragColor = vec4(vertexNormal, 1.0);
}
