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

struct Tile {
    uint count;
    uint offset;
};

layout(std430, binding=0) buffer light_buffer {
    Light lights[];
};

layout(std430, binding = 1) buffer tile_buffer {
    Tile tiles[];
};

layout(std430, binding = 2) buffer tile_light_buffer {
    uint light_indices[];
};

in vec3 vertex_color;
in vec3 vertex_normal;
in vec3 frag_pos;

out vec4 FragColor;

uniform Material material;

uniform vec3 camera_pos;

uniform int light_count;

uniform sampler2D depth_texture;
uniform vec2 screen_size;

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
    uint tiles_x = uint((screen_size.x + 15.0) / 16.0);

    uvec2 tile = uvec2(gl_FragCoord.xy) / 16u;
    uint tile_index = tile.y * tiles_x + tile.x;

    Tile tile_data = tiles[tile_index];

    // if (tile_data.count == 0) {
    //     FragColor = vec4(1, 0, 0, 1);
    //     return;
    // }

    float ambient_strength = 0.3;

    if (tile_data.count > 0)
        ambient_strength /= float(tile_data.count);

    vec3 result = vec3(0.0);

    for (uint i = 0; i < tile_data.count; i++) {
        uint light_index = light_indices[tile_data.offset + i];

        result += blinn_phong_shading(
            lights[light_index],
            ambient_strength
        );
    }

    FragColor = vec4(result, 1.0);
}
