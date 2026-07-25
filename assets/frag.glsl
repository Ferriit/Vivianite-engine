#version 460 core

struct Material {
    vec3 albedo;
    float shininess;
    float specularStrength;
};

struct Light {
    vec4 position;
    vec4 color;
    float radius;
    float strength;
    float linear;
    float quadratic;
};

layout(std430, binding=0) buffer light_buffer {
    Light lights[];
};

in vec3 vertex_color;
in vec3 vertex_normal;
in vec3 frag_pos;

out vec4 FragColor;

uniform Material material;

uniform vec3 camera_pos;

uniform int light_count;

vec3 blinn_phong_shading(Light l, float ambient_strength) {
    vec3 light_position = l.position.xyz;
    vec3 light_color = l.color.rgb * l.strength;

    // Blinn-Phong shading
    float dist = length(frag_pos - light_position);
    if (dist > l.radius)
        return vec3(0.0);

    float attenuation = 1.0 / (1.0 + l.linear * dist + l.quadratic * dist * dist);

    // Diffuse
    vec3 ambient = ambient_strength * light_color * attenuation;
    vec3 N = normalize(vertex_normal);
    vec3 L = normalize(light_position - frag_pos);
    float diff = max(dot(N, L), 0.0);
    vec3 diffuse_col = diff * material.albedo * light_color * attenuation;

    // Specular shading
    vec3 V = normalize(camera_pos - frag_pos);
    vec3 halfway_dir = normalize(L + V);
    float spec = pow(max(dot(N, halfway_dir), 0.0), material.shininess);
    vec3 specular_col = material.specularStrength * spec * light_color * attenuation;

    // Resulting color
    return ambient + diffuse_col + specular_col;
}

void main() {
    float ambient_strength = 0.3;
    if (light_count > 0)
        ambient_strength /= float(light_count);
    
    vec3 result = vec3(0.0);
    for (int i = 0; i < light_count; i++) {
        result += blinn_phong_shading(lights[i], ambient_strength);
    }

    if (light_count == 0) {
        FragColor = vec4(1,0,0,1);
        return;
    }

    FragColor = vec4(result, 1.0);
}
