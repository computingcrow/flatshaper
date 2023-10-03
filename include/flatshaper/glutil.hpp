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

#ifndef FLATSHAPER_GLUTIL_HPP
#define FLATSHAPER_GLUTIL_HPP

#include <glad/glad.h>

#include <vector>
#include <filesystem>


namespace flatshaper {
    GLuint load_model(const std::filesystem::path &ply_file, int32_t &element_count);
    GLuint load_texture(const std::filesystem::path &texture_file);
    GLuint load_shader(const std::filesystem::path &shader_file, GLenum shader_type);
}

#endif
