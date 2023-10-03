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

#include <flatshaper/main.hpp>
#include <flatshaper/systems/render/system_render.hpp>
#include <flatshaper/systems/system_physics.hpp>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <IL/il.h>

#include <stdexcept>


GLFWwindow *create_window();

int main() {
    GLFWwindow *window = create_window();
    ilInit();

    std::filesystem::path assets_directory = std::filesystem::u8path(u8"assets");

    flatshaper::systems::render::render_initialize(assets_directory);

    flatshaper::systems::render::render_load_assets(std::filesystem::absolute(
            assets_directory
            / std::filesystem::u8path(u8"levels")
            / std::filesystem::u8path(u8"lv1")
            / std::filesystem::u8path(u8"assets.csv")));

    flatshaper::systems::render::render_camera_position = glm::vec3(0.0f, 0.0f, -1.0f);
    flatshaper::systems::render::render_camera_direction = glm::vec3(0.0f, 0.0f, 1.0f);
    flatshaper::systems::render::render_screen_width = 800;
    flatshaper::systems::render::render_screen_height = 600;
    flatshaper::systems::render::render_fov = 70.0f;

    entityid_t flat = flatshaper::generate_entity_id();
    flatshaper::systems::physics_position[flat] = glm::vec3(0.0f, 0.0f, 8.0f);
    flatshaper::systems::render::render_add_entity(flat, 6);

    while (!glfwWindowShouldClose(window)) {
        flatshaper::systems::physics_simulate();

        flatshaper::systems::render::render_draw();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

GLFWwindow *create_window() {
    if (!glfwInit()) {
        throw std::runtime_error(u8"Cannot initialise GLFW");
    }

    glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow *window = glfwCreateWindow(800, 600, u8"flatshaper", nullptr, nullptr);

    if (window == nullptr) {
        throw std::runtime_error(u8"Cannot create GLFW window");
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        throw std::runtime_error(u8"Cannot initialise GLAD");
    }

    return window;
}
