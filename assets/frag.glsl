#version 460 core

out vec4 FragColor;
in vec4 gl_FragCoord;

void main() {
    FragColor = vec4(gl_FragCoord, 0., 0.);
}