
#include <flatshaper/systems/render/system_render.hpp>
#include <flatshaper/glutil.hpp>

#include <glad/glad.h>

#include <unordered_set>
#include <unordered_map>
#include <array>
#include <fstream>
#include <charconv>


namespace flatshaper::systems::render {
    enum class ASSET_TYPE : uint8_t {
        MODEL = 0,
        DIFFUSE = 1,
        NORMAL = 2,
        VERTEX = 3,
        FRAGMENT = 4,
        LAST = 5
    };

    // This is essentially read-only after initialization
    struct {
        std::unordered_map<assetid_t, ASSET_TYPE> assets_to_asset_types;
        std::unordered_map<assetid_t, std::filesystem::path> assets_to_files;
    } assets_database;

    struct {
        std::unordered_map<ASSET_TYPE, std::unordered_map<assetid_t, GLuint>> assets_dependencies;
        std::unordered_map<assetid_t, int32_t> assets_to_element_counts;

        std::unordered_map<GLuint, GLuint> shader_programs_to_vertex_shaders;
        std::unordered_map<GLuint, GLuint> shader_programs_to_fragment_shaders;
        std::unordered_map<assetid_t, GLuint> assets_to_shader_programs;

        std::unordered_map<GLuint, GLint> shader_programs_to_model_matrix_uniform_locations;
        std::unordered_map<GLuint, GLint> shader_programs_to_view_matrix_uniform_locations;
        std::unordered_map<GLuint, GLint> shader_programs_to_projection_matrix_uniform_locations;
        std::unordered_map<GLuint, GLint> shader_programs_to_diffuse_texture_uniform_locations;
        std::unordered_map<GLuint, GLint> shader_programs_to_normal_texture_uniform_locations;
    } gl_names;


    void render_initialize(const std::filesystem::path& assets_directory) {
        std::filesystem::path assets_file = std::filesystem::absolute(assets_directory / u8"assets.csv");
        if (!std::filesystem::exists(assets_file))
            throw std::runtime_error(u8"Assets database does not exist");

        std::ifstream assets_file_stream(assets_file);

        std::string line;
        while (std::getline(assets_file_stream, line)) {
            auto idx = line.find(u8';');
            assetid_t assetid;
            if (std::from_chars(line.c_str(), line.c_str() + idx, assetid).ec != std::errc())
                throw std::runtime_error(u8"Invalid asset ID");

            auto start_idx = idx + 1;
            idx = line.find(u8';', start_idx);
            std::string asset_type_string = line.substr(start_idx, idx - start_idx);
            ASSET_TYPE asset_type;
            if (std::equal(asset_type_string.begin(), asset_type_string.end(), u8"MODEL"))
                asset_type = ASSET_TYPE::MODEL;
            else if (std::equal(asset_type_string.begin(), asset_type_string.end(), u8"DIFFUSE"))
                asset_type = ASSET_TYPE::DIFFUSE;
            else if (std::equal(asset_type_string.begin(), asset_type_string.end(), u8"NORMAL"))
                asset_type = ASSET_TYPE::NORMAL;
            else if (std::equal(asset_type_string.begin(), asset_type_string.end(), u8"VERTEX"))
                asset_type = ASSET_TYPE::VERTEX;
            else if (std::equal(asset_type_string.begin(), asset_type_string.end(), u8"FRAGMENT"))
                asset_type = ASSET_TYPE::FRAGMENT;
            else
                throw std::runtime_error(u8"Invalid asset type");

            start_idx = idx + 1;
            std::string asset_file_string = line.substr(start_idx);
            std::filesystem::path asset_file = assets_directory / asset_file_string;

            assets_database.assets_to_asset_types[assetid] = asset_type;
            assets_database.assets_to_files[assetid] = asset_file;
        }

        assets_file_stream.close();
    }

    typedef GLuint (*asset_load_function)(const std::filesystem::path &asset_path);

    void load_asset_type(const std::unordered_map<assetid_t, assetid_t> &assets_to_load,
                         std::unordered_map<assetid_t, GLuint> &assets_to_gl_name_map,
                         asset_load_function load_function) {
        for (const auto &item: assets_to_load) {
            auto x = assets_to_gl_name_map.find(item.second);
            GLuint name;

            if (x == assets_to_gl_name_map.end()) {
                GLuint texture_name = load_function(assets_database.assets_to_files[item.second]);
                name = texture_name;
            } else {
                name = x->second;
            }

            assets_to_gl_name_map[item.first] = name;
        }
    }

    void render_load_assets(const std::filesystem::path& assets_list_file) {
        std::unordered_set<assetid_t> assets_in_file;

        // For a level, parse the list of all assets:
        // - First column: The asset ID of the new asset
        // - Second column: The model asset
        // - Third/fourth column: The diffuse texture map and the normal map
        // - Fifth/sixth column: The vertex/fragment shader
        // For each asset, save for all of the above asset types which asset-id fulfils the dependency
        // Afterwards, load the assets and save which GL-name belongs to the asset's dependency
        // TODO: Unloading can be implemented later, by checking which assets

        std::unordered_map<ASSET_TYPE, std::unordered_map<assetid_t, assetid_t>> asset_dependencies_to_load{};

        std::string line;
        std::ifstream asset_list_file_input_stream(assets_list_file);
        while (std::getline(asset_list_file_input_stream, line)) {
            assetid_t assetids[6]{};

            auto end_idx = line.find(u8';');
            auto start_idx = 0 * end_idx;

            for (int i = 0; i < 5; i++) {
                assetid_t &assetid = assetids[i];
                if (std::from_chars(line.c_str() + start_idx, line.c_str() + end_idx, assetid).ec != std::errc())
                    throw std::runtime_error(u8"Invalid asset ID");

                start_idx = end_idx + 1;
                end_idx = line.find(u8';', start_idx);
            }

            assetid_t &assetid = assetids[5];
            if (std::from_chars(line.c_str() + start_idx, line.c_str() + line.length(), assetid).ec != std::errc())
                throw std::runtime_error(u8"Invalid asset ID");

            asset_dependencies_to_load[ASSET_TYPE::MODEL][assetids[0]] = assetids[1];
            asset_dependencies_to_load[ASSET_TYPE::DIFFUSE][assetids[0]] = assetids[2];
            asset_dependencies_to_load[ASSET_TYPE::NORMAL][assetids[0]] = assetids[3];
            asset_dependencies_to_load[ASSET_TYPE::VERTEX][assetids[0]] = assetids[4];
            asset_dependencies_to_load[ASSET_TYPE::FRAGMENT][assetids[0]] = assetids[5];
        }
        asset_list_file_input_stream.close();

        for (const auto &item: asset_dependencies_to_load[ASSET_TYPE::MODEL]) {
            auto x = gl_names.assets_dependencies[ASSET_TYPE::MODEL].find(item.second);
            GLuint name;

            if (x == gl_names.assets_dependencies[ASSET_TYPE::MODEL].end()) {
                int32_t element_count = 0;
                GLuint vertex_array = load_model(assets_database.assets_to_files[item.second], element_count);
                name = vertex_array;
                gl_names.assets_to_element_counts[item.first] = element_count;
            } else {
                name = x->second;
            }

            gl_names.assets_dependencies[ASSET_TYPE::MODEL][item.first] = name;
        }

        load_asset_type(asset_dependencies_to_load[ASSET_TYPE::DIFFUSE], gl_names.assets_dependencies[ASSET_TYPE::DIFFUSE], load_texture);
        load_asset_type(asset_dependencies_to_load[ASSET_TYPE::NORMAL], gl_names.assets_dependencies[ASSET_TYPE::NORMAL], load_texture);
        load_asset_type(asset_dependencies_to_load[ASSET_TYPE::VERTEX], gl_names.assets_dependencies[ASSET_TYPE::VERTEX], [](const std::filesystem::path &path) { return load_shader(path, GL_VERTEX_SHADER); });
        load_asset_type(asset_dependencies_to_load[ASSET_TYPE::FRAGMENT], gl_names.assets_dependencies[ASSET_TYPE::FRAGMENT], [](const std::filesystem::path &path) { return load_shader(path, GL_FRAGMENT_SHADER); });

        for (const auto &asset_to_vertex_shader: gl_names.assets_dependencies[ASSET_TYPE::VERTEX]) {
            GLuint vertex_shader = asset_to_vertex_shader.second;
            GLuint fragment_shader = gl_names.assets_dependencies[ASSET_TYPE::FRAGMENT][asset_to_vertex_shader.first];

            // If possible, find a shader program that already uses the same vertex and fragment shader
            GLuint shader_program = 0;
            for (const auto &shader_program_to_vertex_shader: gl_names.shader_programs_to_vertex_shaders) {
                if (shader_program_to_vertex_shader.second == vertex_shader) {
                    auto x = gl_names.shader_programs_to_fragment_shaders.find(shader_program_to_vertex_shader.first);
                    if (x != gl_names.shader_programs_to_fragment_shaders.end() && x->second == fragment_shader)
                        shader_program = shader_program_to_vertex_shader.first;
                }
            }

            // Otherwise, create a new program and link it
            if (shader_program == 0) {
                shader_program = glCreateProgram();
                glAttachShader(shader_program, vertex_shader);
                glAttachShader(shader_program, fragment_shader);
                glLinkProgram(shader_program);

                gl_names.shader_programs_to_vertex_shaders[shader_program] = vertex_shader;
                gl_names.shader_programs_to_fragment_shaders[shader_program] = fragment_shader;

                auto model_location = glGetUniformLocation(shader_program, u8"um_model");
                auto view_location = glGetUniformLocation(shader_program, u8"um_view");
                auto projection_location = glGetUniformLocation(shader_program, u8"um_projection");
                auto diffuse_location = glGetUniformLocation(shader_program, u8"ut_diffuse");
                auto normal_location = glGetUniformLocation(shader_program, u8"ut_normal");

                gl_names.shader_programs_to_model_matrix_uniform_locations[shader_program] = model_location;
                gl_names.shader_programs_to_view_matrix_uniform_locations[shader_program] = view_location;
                gl_names.shader_programs_to_projection_matrix_uniform_locations[shader_program] = projection_location;
                gl_names.shader_programs_to_diffuse_texture_uniform_locations[shader_program] = diffuse_location;
                gl_names.shader_programs_to_normal_texture_uniform_locations[shader_program] = normal_location;
            }

            gl_names.assets_to_shader_programs[asset_to_vertex_shader.first] = shader_program;
        }
        // TODO: Implement unloading
    }
}
