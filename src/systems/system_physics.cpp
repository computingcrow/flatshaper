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

#include <flatshaper/systems/system_physics.hpp>

#include <glm/ext/matrix_transform.hpp>

namespace flatshaper::systems {
    std::unordered_map<entityid_t, glm::vec3> physics_position;
    std::unordered_map<entityid_t, glm::mat4> physics_matrix;

    void physics_simulate() {
        for (const auto &item: physics_position) {
            physics_matrix[item.first] = glm::translate(glm::identity<glm::mat4>(), item.second);
        }
    }
}
