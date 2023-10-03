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

#ifndef FLATSHAPER_SYSTEMS_SYSTEM_PHYSICS_HPP
#define FLATSHAPER_SYSTEMS_SYSTEM_PHYSICS_HPP

#include <flatshaper/entity.hpp>

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

#include <unordered_map>


namespace flatshaper::systems {
    extern std::unordered_map<entityid_t, glm::vec3> physics_position;
    extern std::unordered_map<entityid_t, glm::mat4> physics_matrix;

    void physics_simulate();
}

#endif
