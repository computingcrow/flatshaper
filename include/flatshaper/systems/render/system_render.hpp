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

#ifndef FLATSHAPER_SYSTEMS_SYSTEM_RENDER_HPP
#define FLATSHAPER_SYSTEMS_SYSTEM_RENDER_HPP

#include "flatshaper/entity.hpp"

#include <glm/vec3.hpp>

#include <filesystem>


typedef std::uint32_t assetid_t;

namespace flatshaper::systems::render {
    extern glm::vec3 render_camera_position;
    extern glm::vec3 render_camera_direction;
    extern float render_screen_width;
    extern float render_screen_height;
    extern float render_fov;

    void render_initialize(const std::filesystem::path& assets_directory);

    void render_load_assets(const std::filesystem::path& assets_list_file);
    void render_add_entity(entityid_t entity, assetid_t assetid);
    void render_remove_entity(entityid_t entityid);
    void render_draw();
}

#endif
