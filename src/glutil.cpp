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

#include <flatshaper/glutil.hpp>
#include <flatshaper/plyutil.hpp>

#include <IL/il.h>

#include <fstream>

#ifdef FLATSHAPER_DEBUG_GL
#define gl_clear_errors() while (int err = glGetError())
#define gl_fail_on_gl_error() if (int err = glGetError()) throw std::runtime_error(std::string(u8"OpenGL error ") + std::to_string(err))
#else
#define gl_clear_errors()
#define gl_fail_on_gl_error()
#endif


namespace flatshaper {
    GLuint load_model(const std::filesystem::path &ply_file, int32_t &element_count) {
        std::vector<float> vertex_data;
        std::vector<uint32_t> element_data;
        parse_ply(ply_file, vertex_data, element_data);
        uint32_t element_count_u = element_data.size();
        if (element_count_u > std::numeric_limits<int32_t>::max()) {
            throw std::runtime_error(u8"Way too large a model. Workshop this.");
        }

        element_count = (int32_t) element_count_u;

        gl_clear_errors();
        gl_fail_on_gl_error();

        GLuint vertex_array = 0;
        GLuint data_buffers[2]{};
        glGenVertexArrays(1, &vertex_array);
        gl_fail_on_gl_error();
        glBindVertexArray(vertex_array);
        gl_fail_on_gl_error();
        glGenBuffers(2, data_buffers);
        gl_fail_on_gl_error();
        glBindBuffer(GL_ARRAY_BUFFER, data_buffers[0]);
        gl_fail_on_gl_error();
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data_buffers[1]);
        gl_fail_on_gl_error();
        glBufferData(GL_ARRAY_BUFFER, (long) (vertex_data.size() * sizeof(float)), vertex_data.data(), GL_STATIC_DRAW);
        gl_fail_on_gl_error();
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, (long) (element_data.size() * sizeof(float)), element_data.data(),
                     GL_STATIC_DRAW);
        gl_fail_on_gl_error();

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), nullptr);
        gl_fail_on_gl_error();
        glEnableVertexAttribArray(0);
        gl_fail_on_gl_error();
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), ((void *) (3 * sizeof(float))));
        gl_fail_on_gl_error();
        glEnableVertexAttribArray(1);
        gl_fail_on_gl_error();

        return vertex_array;
    }

    GLuint load_texture(const std::filesystem::path &texture_file) {
        ILuint image_name = ilGenImage();
        auto file = std::filesystem::absolute(texture_file);
        ilBindImage(image_name);
        ilLoadImage(file.c_str());

        ILint image_width = ilGetInteger(IL_IMAGE_WIDTH);
        ILint image_height = ilGetInteger(IL_IMAGE_HEIGHT);
        ILint image_format = ilGetInteger(IL_IMAGE_FORMAT);
        ILint image_type = ilGetInteger(IL_IMAGE_TYPE);

        GLint gl_texture_format;
        GLint gl_texture_type;
        switch (image_type) {
            case IL_BYTE:
            case IL_UNSIGNED_BYTE:
                if (image_format == IL_LUMINANCE) {
                    gl_texture_format = GL_RED;
                } else if (image_format == IL_RGB) {
                    gl_texture_format = GL_RGB;
                } else if (image_format == IL_RGBA) {
                    gl_texture_format = GL_RGBA;
                } else {
                    throw std::runtime_error(u8"Invalid image format");
                }

                gl_texture_type = GL_UNSIGNED_BYTE;
                break;
            case IL_INT:
            case IL_UNSIGNED_INT:
                if (image_format == IL_RGBA) {
                    gl_texture_format = GL_RGBA;
                } else {
                    throw std::runtime_error(u8"Invalid image format");
                }

                gl_texture_type = GL_UNSIGNED_INT_8_8_8_8;
                break;
            default:
                throw std::runtime_error(u8"Invalid image data type");
        }

        ILubyte *data = ilGetData();

        GLuint texture_name;
        glGenTextures(1, &texture_name);
        gl_fail_on_gl_error();
        glBindTexture(GL_TEXTURE_2D, texture_name);
        gl_fail_on_gl_error();
        glTexImage2D(GL_TEXTURE_2D,
                     0,
                     gl_texture_format,
                     image_width,
                     image_height,
                     0,
                     gl_texture_format,
                     gl_texture_type,
                     data);
        gl_fail_on_gl_error();
        glGenerateMipmap(GL_TEXTURE_2D);
        gl_fail_on_gl_error();
        glBindTexture(GL_TEXTURE_2D, 0);

        ilBindImage(0);
        ilDeleteImage(image_name);

        return texture_name;
    }

    GLuint load_shader(const std::filesystem::path &shader_file, GLenum shader_type) {
        uintmax_t file_size = std::filesystem::file_size(shader_file);
        if (file_size > 32 * 1024 * 1024) {
            throw std::runtime_error(u8"Invalid shader (file too large, possibly wrong file");
        }

        int file_size_i = ((int) file_size);
        std::vector<uint8_t> shader_text(file_size_i + 1);

        std::ifstream shader_input_stream(shader_file);
        if (!shader_input_stream.read((std::ifstream::char_type *) shader_text.data(), file_size_i))
            throw std::runtime_error(u8"Cannot read shader file");
        shader_input_stream.close();

        GLuint shader_name = glCreateShader(shader_type);
        gl_fail_on_gl_error();
        uint8_t *shader_text_data = shader_text.data();
        GLchar **shader_text_data_ptr = ((GLchar **) &shader_text_data);
        glShaderSource(shader_name, 1, shader_text_data_ptr, nullptr);
        gl_fail_on_gl_error();
        glCompileShader(shader_name);
        gl_fail_on_gl_error();

        shader_text.clear();

        return shader_name;
    }
}
