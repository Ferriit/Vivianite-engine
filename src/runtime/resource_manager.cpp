#include "include.hpp"

namespace vivianite {
    bool ResourceManager::obj_load_obj(ResourceID ID) {
        if (is_loaded(ID))
            return true;

        auto [loaded, obj, path] = objects[ID];

        Logging().log(Logging::INFO, "Reading file \"{}\"", path);

        std::ifstream file(path);

        if (!file.is_open()) {
            Logging().log(Logging::ERROR, "Unable to open model \"{}\"", path);
            return false;
        }

        std::vector<float> x_list;
        std::vector<float> y_list;
        std::vector<float> z_list;

        std::vector<float> nx_list;
        std::vector<float> ny_list;
        std::vector<float> nz_list;

        std::vector<float> u_list;
        std::vector<float> v_list;

        struct Face {
            std::array<int, 3> vertex;
            std::array<int, 3> normal;
            std::array<int, 3> UV;
        };

        std::vector<Face> faces;

        std::string line;

        while (std::getline(file, line)) {
            if (line.rfind("v ", 0) == 0) {
                std::stringstream ss(line.substr(2));

                float x, y, z;
                ss >> x >> y >> z;

                x_list.push_back(x);
                y_list.push_back(y);
                z_list.push_back(z);
            }

            else if (line.rfind("vn ", 0) == 0) {
                std::stringstream ss(line.substr(3));

                float nx, ny, nz;
                ss >> nx >> ny >> nz;

                nx_list.push_back(nx);
                ny_list.push_back(ny);
                nz_list.push_back(nz);
            }

            else if (line.rfind("f ", 0) == 0) {
                std::stringstream ss(line.substr(2));

                Face face;

                for (int i = 0; i < 3; i++) {
                    std::string token;
                    ss >> token;

                    size_t s1 = token.find('/');
                    size_t s2 = token.find('/', s1 + 1);

                    face.vertex[i] = std::stoi(
                        token.substr(0, s1)
                    ) - 1;

                    face.UV[i] = std::stoi(
                        token.substr(s1 + 1, s2 - s1 - 1)
                    ) - 1;

                    face.normal[i] = std::stoi(
                        token.substr(s2 + 1)
                    ) - 1;
                }

                faces.push_back(face);
            }
            else if (line.rfind("vt ", 0) == 0) {
                std::stringstream ss(line.substr(3));

                float u, v;
                ss >> u >> v;

                u_list.push_back(u);
                v_list.push_back(v);
            }
        }

        if (x_list.empty())
            return false;

        float min_x = x_list[0], max_x = x_list[0];
        float min_y = y_list[0], max_y = y_list[0];
        float min_z = z_list[0], max_z = z_list[0];

        for (size_t i = 1; i < x_list.size(); i++) {
            min_x = std::min(min_x, x_list[i]);
            max_x = std::max(max_x, x_list[i]);

            min_y = std::min(min_y, y_list[i]);
            max_y = std::max(max_y, y_list[i]);

            min_z = std::min(min_z, z_list[i]);
            max_z = std::max(max_z, z_list[i]);
        }

        float cx = (min_x + max_x) * 0.5f;
        float cy = (min_y + max_y) * 0.5f;
        float cz = (min_z + max_z) * 0.5f;

        float dx = (max_x - min_x == 0) ? 1.0f : max_x - min_x;
        float dy = (max_y - min_y == 0) ? 1.0f : max_y - min_y;
        float dz = (max_z - min_z == 0) ? 1.0f : max_z - min_z;

        std::vector<float> vertices;
        vertices.reserve(faces.size() * 3 * 11);

        for (const Face& face : faces) {
            for (int i = 0; i < 3; i++) {

                int vi = face.vertex[i];
                int ni = face.normal[i];
                int ti = face.UV[i];

                float x = x_list[vi];
                float y = y_list[vi];
                float z = z_list[vi];

                float r = (x - min_x) / dx;
                float g = (y - min_y) / dy;
                float b = (z - min_z) / dz;

                x -= cx;
                y -= cy;
                z -= cz;

                // Position
                vertices.push_back(x);
                vertices.push_back(y);
                vertices.push_back(z);

                // Color
                vertices.push_back(r);
                vertices.push_back(g);
                vertices.push_back(b);

                // Normal
                vertices.push_back(nx_list[ni]);
                vertices.push_back(ny_list[ni]);
                vertices.push_back(nz_list[ni]);

                // UV
                vertices.push_back(u_list[ti]);
                vertices.push_back(v_list[ti]);
            }
        }

        objects[ID] = std::tuple(true, 
            std::static_pointer_cast<void>(
                std::make_shared<mesh>(
                    std::move(vertices),
                    0,
                    faces.size() * 3
                )
            ),
            path
        );

        return true;
    }

    bool ResourceManager::tex_load_obj(ResourceID ID) {
        if (is_loaded(ID))
            return true;

        auto [loaded, obj, path] = objects[ID];

        int width, height, channels;
        stbi_set_flip_vertically_on_load(true);
        unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels, 0);

        if (!data)
            return false;

        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);

        Texture tex_template = *std::reinterpret_pointer_cast<Texture>(this->data[ID]);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, tex_template.wraping);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, tex_template.wraping);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, tex_template.min_filtering);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, tex_template.mag_filtering);

        GLenum format = GL_RGB;

        switch (channels) {
            case 1: format = GL_RED; break;
            case 2: format = GL_RG; break;
            case 3: format = GL_RGB; break;
            case 4: format = GL_RGBA; break;
        }

        GLenum internal_format = GL_RGB8;

        switch (channels) {
            case 1: internal_format = GL_R8; break;
            case 2: internal_format = GL_RG8; break;
            case 3: internal_format = GL_RGB8; break;
            case 4: internal_format = GL_RGBA8; break;
        }

        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            internal_format,
            width,
            height,
            0,
            format,
            GL_UNSIGNED_BYTE,
            data
        );

        glGenerateMipmap(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0);

        stbi_image_free(data);

        auto tex = std::make_shared<Texture>();

        tex->wraping = tex_template.wraping;
        tex->min_filtering = tex_template.min_filtering;
        tex->mag_filtering = tex_template.mag_filtering;
        tex->texture = texture;

        objects[ID] = {
            true,
            std::static_pointer_cast<void>(tex),
            path
        };

        return true;
    }
};
