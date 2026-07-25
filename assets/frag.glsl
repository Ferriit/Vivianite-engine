#version 460 core

struct Material {
    vec3 albedo;
    float shininess;
    float specularStrength;
};

in vec3 vertex_color;
in vec3 vertex_normal;
in vec3 frag_pos;

out vec4 FragColor;

uniform Material material;
uniform vec3 light_pos;
uniform vec3 light_col;

uniform vec3 camera_pos;

void main() {
    float ambient_strength = 0.7;

    // Blinn-Phong shading
    // Diffuse
    vec3 ambient = ambient_strength * light_col;
    vec3 N = normalize(vertex_normal);
    vec3 L = normalize(light_pos - frag_pos);
    float diff = max(dot(N, L), 0.0);
    vec3 diffuse_col = diff * material.albedo * light_col;

    // Specular shading
    vec3 V = normalize(camera_pos - frag_pos);
    vec3 halfway_dir = normalize(L + V);
    float spec = pow(max(dot(N, halfway_dir), 0.0), material.shininess);
    vec3 specular_col = material.specularStrength * spec * light_col;

    // Resulting color
    vec3 result_col = ambient + diffuse_col + specular_col;

    FragColor = vec4(result_col, 1.0);
}
