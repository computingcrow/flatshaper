// flatshaper, a small video game
// Copyright (C) 2023  computingcrow
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

#include <flatshaper/systems/render/system_render.hpp>
#include <flatshaper/systems/system_physics.hpp>
#include "render_assets.cpp"

#include <glad/glad.h>
#include <glm/ext.hpp>

#include <unordered_map>
#include <vector>


namespace flatshaper::systems::render {
    glm::vec3 render_camera_position{};
    glm::vec3 render_camera_direction{};
    float render_screen_width{};
    float render_screen_height{};
    float render_fov{};

    std::unordered_map<entityid_t, assetid_t> rendered_entities;

    glm::mat4 view_matrix = glm::identity<glm::mat4>();
    glm::mat4 projection_matrix = glm::identity<glm::mat4>();

    void render_add_entity(entityid_t entity, assetid_t assetid) {
        if (is_entity_valid(entity)) {
            rendered_entities[entity] = assetid;
        }
    }

    void render_remove_entity(entityid_t entityid) {
        rendered_entities.erase(entityid);
    }

    void render_draw() {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);

        projection_matrix = glm::perspective(render_fov, render_screen_width / render_screen_height, 0.1f, 10.1f);
        view_matrix = glm::lookAt(render_camera_position, render_camera_position + render_camera_direction, glm::vec3(0.0f, 1.0f, 0.0f));

        std::unordered_map<assetid_t, std::vector<glm::mat4x4>> assets_to_matrices;

        for (const auto &rendered_entity: rendered_entities) {
            if (assets_to_matrices.find(rendered_entity.second) == assets_to_matrices.end()) {
                assets_to_matrices[rendered_entity.second] = std::vector<glm::mat4x4>{};
            }

            auto position = physics_matrix.find(rendered_entity.first);
            if (position == physics_matrix.end()) {
                rendered_entities.erase(rendered_entity.first);
                continue;
            }

            std::vector<glm::mat4x4> &positions = assets_to_matrices[rendered_entity.second];
            positions.push_back(position->second);
        }

        for (const auto &asset_to_matrix: assets_to_matrices) {
            GLuint model = gl_names.assets_dependencies[ASSET_TYPE::MODEL][asset_to_matrix.first];
            GLuint shader_program = gl_names.assets_to_shader_programs[asset_to_matrix.first];
            int32_t element_count = gl_names.assets_to_element_counts[asset_to_matrix.first];
            GLuint diffuse_texture = gl_names.assets_dependencies[ASSET_TYPE::DIFFUSE][asset_to_matrix.first];
            GLuint normal_texture = gl_names.assets_dependencies[ASSET_TYPE::NORMAL][asset_to_matrix.first];

            GLint model_matrix_uniform_location = gl_names.shader_programs_to_model_matrix_uniform_locations[shader_program];
            GLint view_matrix_uniform_location = gl_names.shader_programs_to_view_matrix_uniform_locations[shader_program];
            GLint projection_matrix_uniform_location = gl_names.shader_programs_to_projection_matrix_uniform_locations[shader_program];
            GLint diffuse_texture_uniform_location = gl_names.shader_programs_to_diffuse_texture_uniform_locations[shader_program];
            GLint normal_texture_uniform_location = gl_names.shader_programs_to_normal_texture_uniform_locations[shader_program];

            glUseProgram(shader_program);

            glUniform1i(diffuse_texture_uniform_location, 0);
            glUniform1i(normal_texture_uniform_location, 1);
            glUniformMatrix4fv(projection_matrix_uniform_location, 1, GL_FALSE, &projection_matrix[0][0]);
            glUniformMatrix4fv(view_matrix_uniform_location, 1, GL_FALSE, &view_matrix[0][0]);


            glBindVertexArray(model);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, diffuse_texture);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, normal_texture);
            glActiveTexture(GL_TEXTURE0);

            for (const auto &matrix: asset_to_matrix.second) {
                glUniformMatrix4fv(model_matrix_uniform_location, 1, GL_FALSE, &matrix[0][0]);

                glDrawElements(GL_TRIANGLES, element_count, GL_UNSIGNED_INT, nullptr);
            }
        }
    }
}
