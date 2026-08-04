#version 460 core

struct Material {
    float shininess;
    float specularStrength;
};

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 UV;

out vec3 frag_pos;
out vec3 vertex_normal;
out vec3 vertex_color;
out vec2 vertex_UV;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform Material material;

void main() {
    vec4 world_pos = model * vec4(position, 1.0);

    frag_pos = world_pos.xyz;

    vertex_normal = normalize(mat3(transpose(inverse(model))) * normal);

    vertex_color = color;

    vertex_UV = UV;

    gl_Position = projection * view * world_pos;
}
