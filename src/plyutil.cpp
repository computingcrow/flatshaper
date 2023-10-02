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

#include <fstream>
#include <charconv>

#include <flatshaper/plyutil.hpp>

namespace flatshaper {
    std::ifstream &read_next_line(std::ifstream &input_stream, std::string &line) {
        if (!std::getline(input_stream, line))
            throw std::runtime_error(u8"Cannot read PLY input stream");

        while (line.find(u8"comment") != std::string::npos) {
            if (!std::getline(input_stream, line))
                throw std::runtime_error(u8"Cannot read PLY input stream");
        }

        return input_stream;
    }

    template<typename T>
    T two_bytes_to_numeral(uint8_t byte_0, uint8_t byte_1) {
        static_assert(sizeof(T) == 2, u8"Numeral type is not 2 bytes");
        uint8_t byte_array[2] = {byte_0, byte_1};
        T out = 0;
        std::copy(byte_array, byte_array + 1, (uint8_t *) (&out));
        return out;
    }

    template<typename T>
    T four_bytes_to_numeral(uint8_t byte_0, uint8_t byte_1, uint8_t byte_2, uint8_t byte_3) {
        static_assert(sizeof(T) == 4, u8"Numeral type is not 4 bytes");
        T out = 0;
        (reinterpret_cast<uint8_t *>(&out))[0] = byte_0;
        (reinterpret_cast<uint8_t *>(&out))[1] = byte_1;
        (reinterpret_cast<uint8_t *>(&out))[2] = byte_2;
        (reinterpret_cast<uint8_t *>(&out))[3] = byte_3;
        return out;
    }

    bool parse_ply_binary(std::ifstream &ply_input_stream,
                          std::vector<float> &vertex_data,
                          std::vector<uint32_t> &element_data,
                          uint32_t vertex_count,
                          uint32_t face_count,
                          uint32_t property_list_vertex_indices_count_type,
                          uint32_t property_list_vertex_indices_index_type,
                          uint32_t property_count,
                          uint32_t property_x_index,
                          uint32_t property_y_index,
                          uint32_t property_z_index,
                          uint32_t property_s_index,
                          uint32_t property_t_index) {
        std::vector<uint8_t> buffer_bytes(property_count * sizeof(float));
        for (uint32_t i = 0; i < vertex_count; i++) {
            if (!ply_input_stream.read((std::ifstream::char_type *) buffer_bytes.data(), (long) buffer_bytes.size())) {
                throw std::runtime_error(u8"Malformed PLY file (EOF?)");
            }

            uint8_t *property_x_bytes = &buffer_bytes[property_x_index * 4];
            uint8_t *property_y_bytes = &buffer_bytes[property_y_index * 4];
            uint8_t *property_z_bytes = &buffer_bytes[property_z_index * 4];
            uint8_t *property_s_bytes = &buffer_bytes[property_s_index * 4];
            uint8_t *property_t_bytes = &buffer_bytes[property_t_index * 4];

            auto x_value = four_bytes_to_numeral<float>(property_x_bytes[0], property_x_bytes[1], property_x_bytes[2], property_x_bytes[3]);
            auto y_value = four_bytes_to_numeral<float>(property_y_bytes[0], property_y_bytes[1], property_y_bytes[2], property_y_bytes[3]);
            auto z_value = four_bytes_to_numeral<float>(property_z_bytes[0], property_z_bytes[1], property_z_bytes[2], property_z_bytes[3]);
            auto s_value = four_bytes_to_numeral<float>(property_s_bytes[0], property_s_bytes[1], property_s_bytes[2], property_s_bytes[3]);
            auto t_value = four_bytes_to_numeral<float>(property_t_bytes[0], property_t_bytes[1], property_t_bytes[2], property_t_bytes[3]);

            vertex_data.push_back(x_value);
            vertex_data.push_back(y_value);
            vertex_data.push_back(z_value);
            vertex_data.push_back(s_value);
            vertex_data.push_back(t_value);
        }

        buffer_bytes.resize(property_list_vertex_indices_count_type + 3 * property_list_vertex_indices_index_type);
        for (uint32_t i = 0; i < face_count; i++) {
            if (!ply_input_stream.read((std::ifstream::char_type *) buffer_bytes.data(), (long) buffer_bytes.size())) {
                throw std::runtime_error(u8"Malformed PLY file (EOF?)");
            }

            uint8_t *face_vertex_count_bytes = &buffer_bytes[0];
            uint8_t *vertex_bytes[3] {};
            vertex_bytes[0] = &buffer_bytes[property_list_vertex_indices_count_type];
            vertex_bytes[1] = &buffer_bytes[property_list_vertex_indices_count_type + property_list_vertex_indices_index_type];
            vertex_bytes[2] = &buffer_bytes[property_list_vertex_indices_count_type + 2 * property_list_vertex_indices_index_type];

            uint32_t face_vertex_count;
            if (property_list_vertex_indices_count_type == 1) {
                face_vertex_count = *face_vertex_count_bytes;
            } else if (property_list_vertex_indices_count_type == 2) {
                face_vertex_count = two_bytes_to_numeral<uint16_t>(face_vertex_count_bytes[0], face_vertex_count_bytes[1]);
            } else {
                face_vertex_count = four_bytes_to_numeral<uint32_t>(face_vertex_count_bytes[0], face_vertex_count_bytes[1], face_vertex_count_bytes[2], face_vertex_count_bytes[3]);
            }

            if (face_vertex_count != 3) {
                throw std::runtime_error(u8"Malformed PLY file (face doesn't have exactly 3 vertices)");
            }

            uint32_t face_vertices[3] {};
            if (property_list_vertex_indices_index_type == 1) {
                face_vertices[0] = *vertex_bytes[0];
                face_vertices[1] = *vertex_bytes[1];
                face_vertices[2] = *vertex_bytes[2];
            } else if (property_list_vertex_indices_index_type == 2) {
                face_vertices[0] = two_bytes_to_numeral<uint16_t>(vertex_bytes[0][0], vertex_bytes[0][1]);
                face_vertices[1] = two_bytes_to_numeral<uint16_t>(vertex_bytes[1][0], vertex_bytes[1][1]);
                face_vertices[2] = two_bytes_to_numeral<uint16_t>(vertex_bytes[2][0], vertex_bytes[2][1]);
            } else {
                face_vertices[0] = four_bytes_to_numeral<uint32_t>(vertex_bytes[0][0], vertex_bytes[0][1], vertex_bytes[0][2], vertex_bytes[0][3]);
                face_vertices[1] = four_bytes_to_numeral<uint32_t>(vertex_bytes[1][0], vertex_bytes[1][1], vertex_bytes[1][2], vertex_bytes[1][3]);
                face_vertices[2] = four_bytes_to_numeral<uint32_t>(vertex_bytes[2][0], vertex_bytes[2][1], vertex_bytes[2][2], vertex_bytes[2][3]);
            }

            element_data.push_back(face_vertices[0]);
            element_data.push_back(face_vertices[1]);
            element_data.push_back(face_vertices[2]);
        }

        return true;
    }

    bool parse_ply_ascii(std::ifstream &ply_input_stream,
                         std::vector<float> &vertex_data,
                         std::vector<uint32_t> &element_data,
                         uint32_t vertex_count,
                         uint32_t face_count,
                         uint32_t property_count,
                         uint32_t property_x_index,
                         uint32_t property_y_index,
                         uint32_t property_z_index,
                         uint32_t property_s_index,
                         uint32_t property_t_index) {
        std::string ignored;
        for (uint32_t i = 0; i < vertex_count; i++) {
            float x = 0, y = 0, z = 0;
            float s = 0, t = 0;

            for (int p = 0; p < property_count; p++) {
                if (p == property_x_index)
                    ply_input_stream >> x;
                else if (p == property_y_index)
                    ply_input_stream >> y;
                else if (p == property_z_index)
                    ply_input_stream >> z;
                else if (p == property_s_index)
                    ply_input_stream >> s;
                else if (p == property_t_index)
                    ply_input_stream >> t;
                else
                    ply_input_stream >> ignored;
            }

            vertex_data.push_back(x);
            vertex_data.push_back(y);
            vertex_data.push_back(z);
            vertex_data.push_back(s);
            vertex_data.push_back(t);
        }

        for (uint32_t i = 0; i < face_count; i++) {
            uint32_t face_vertex_count = 0;
            uint32_t face_first_vertex = 0, face_second_vertex = 0, face_third_vertex = 0;

            ply_input_stream >> face_vertex_count;
            if (face_vertex_count != 3)
                throw std::runtime_error(u8"Malformed PLY file (face doesn't have exactly 3 vertices)");

            ply_input_stream >> face_first_vertex;
            ply_input_stream >> face_second_vertex;
            ply_input_stream >> face_third_vertex;

            element_data.push_back(face_first_vertex);
            element_data.push_back(face_second_vertex);
            element_data.push_back(face_third_vertex);
        }

        return true;
    }

    bool parse_ply(const std::filesystem::path &ply_file,
                   std::vector<float> &vertex_data,
                   std::vector<uint32_t> &element_data) {
        std::ifstream ply_input_stream(ply_file);

        bool header_read = false;
        bool type_read = false;
        bool is_ascii = false;
        bool vertex_count_read = false;
        uint32_t vertex_count = 0;
        bool property_x_read = false;
        uint32_t property_x_index = 0;
        bool property_y_read = false;
        uint32_t property_y_index = 0;
        bool property_z_read = false;
        uint32_t property_z_index = 0;
        bool property_s_read = false;
        uint32_t property_s_index = 0;
        bool property_t_read = false;
        uint32_t property_t_index = 0;
        uint32_t property_count = 0;
        bool face_count_read = false;
        uint32_t face_count = 0;
        uint32_t property_list_vertex_indices_count_type = 0;
        uint32_t property_list_vertex_indices_index_type = 0;
        bool property_list_vertex_indices_read = false;
        bool end_header_read = false;

        std::string line;
        while (read_next_line(ply_input_stream, line)) {
            if (!header_read) {
                if (!std::equal(line.begin(), line.end(), u8"ply")) {
                    throw std::runtime_error(u8"File is not a PLY file");
                } else {
                    header_read = true;
                    continue;
                }
            }

            if (header_read && !type_read) {
                if (std::equal(line.begin(), line.end(), u8"format binary_little_endian 1.0")) {
                    is_ascii = false;
                    type_read = true;
                } else if (std::equal(line.begin(), line.end(), "format ascii 1.0")) {
                    is_ascii = true;
                    type_read = true;
                } else {
                    throw std::runtime_error(u8"Invalid PLY file (invalid/no format)");
                }

                continue;
            }

            if (type_read && !vertex_count_read) {
                if (line.find(u8"element vertex") == std::string::npos) {
                    throw std::runtime_error(u8"Invalid PLY file (element vertex not available)");
                }

                auto whitespace_before_count_index = line.rfind(u8' ');
                if (std::from_chars(line.c_str() + whitespace_before_count_index + 1, line.c_str() + line.length(), vertex_count).ec != std::errc())
                    throw std::runtime_error(u8"Invalid vertex count");

                vertex_count_read = true;
                continue;
            }

            if (vertex_count_read && !face_count_read) {
                if (line.find(u8"property") != std::string::npos) {
                    // Limitation: We expect all properties to be 4 bytes long in binary representation,
                    // thus, we reject non-4-byte-properties for all PLY files
                    if (line.find(u8"float") == std::string::npos &&
                        line.find(u8"uint") == std::string::npos &&
                        line.find(u8"int") == std::string::npos)
                        throw std::runtime_error(u8"Cannot parse PLY file: non-4-byte type used");

                    if (std::equal(line.begin(), line.end(), u8"property float x")) {
                        if (property_x_read)
                            throw std::runtime_error(u8"Invalid PLY file (property defined multiple times)");

                        property_x_read = true;
                        property_x_index = property_count;
                    } else if (std::equal(line.begin(), line.end(), u8"property float y")) {
                        if (property_y_read)
                            throw std::runtime_error(u8"Invalid PLY file (property defined multiple times)");

                        property_y_read = true;
                        property_y_index = property_count;
                    } else if (std::equal(line.begin(), line.end(), u8"property float z")) {
                        if (property_z_read)
                            throw std::runtime_error(u8"Invalid PLY file (property defined multiple times)");

                        property_z_read = true;
                        property_z_index = property_count;
                    } else if (std::equal(line.begin(), line.end(), u8"property float s")) {
                        if (property_s_read)
                            throw std::runtime_error(u8"Invalid PLY file (property defined multiple times)");

                        property_s_read = true;
                        property_s_index = property_count;
                    } else if (std::equal(line.begin(), line.end(), u8"property float t")) {
                        if (property_t_read)
                            throw std::runtime_error(u8"Invalid PLY file (property defined multiple times)");

                        property_t_read = true;
                        property_t_index = property_count;
                    }

                    property_count++;
                    continue;
                }

                if (line.find(u8"element face") != std::string::npos) {
                    auto whitespace_before_count_index = line.rfind(u8' ');
                    if (std::from_chars(line.c_str() + whitespace_before_count_index + 1, line.c_str() + line.length(), face_count).ec != std::errc())
                        throw std::runtime_error(u8"Invalid vertex count");

                    face_count_read = true;
                    continue;
                }
            }

            if (face_count_read && !property_list_vertex_indices_read) {
                if (line.find(u8"property list") != std::string::npos &&
                    line.rfind(u8"vertex_indices") != std::string::npos) {
                    auto first_type_whitespace = line.find(u8' ', sizeof(u8"property list") - 1);
                    auto after_first_type_whitespace = line.find(u8' ', first_type_whitespace + 1);
                    std::string first_type = line.substr(first_type_whitespace + 1, after_first_type_whitespace - first_type_whitespace - 1);
                    if (std::equal(first_type.begin(), first_type.end(), u8"uchar"))
                        property_list_vertex_indices_count_type = 1;
                    else if (std::equal(first_type.begin(), first_type.end(), u8"ushort"))
                        property_list_vertex_indices_count_type = 2;
                    else if (std::equal(first_type.begin(), first_type.end(), u8"uint"))
                        property_list_vertex_indices_count_type = 4;
                    else
                        throw std::runtime_error(u8"Invalid PLY file (unknown type for index count)");

                    auto after_second_type_whitespace = line.rfind(u8' ');
                    std::string second_type = line.substr(after_first_type_whitespace + 1, after_second_type_whitespace - after_first_type_whitespace - 1);
                    if (std::equal(second_type.begin(), second_type.end(), u8"uchar"))
                        property_list_vertex_indices_index_type = 1;
                    else if (std::equal(second_type.begin(), second_type.end(), u8"ushort"))
                        property_list_vertex_indices_index_type = 2;
                    else if (std::equal(second_type.begin(), second_type.end(), u8"uint"))
                        property_list_vertex_indices_index_type = 4;
                    else
                        throw std::runtime_error(u8"Invalid PLY file (unknown type for index type)");

                    property_list_vertex_indices_read = true;
                }

                continue;
            }

            if (property_list_vertex_indices_read && !end_header_read) {
                if (std::equal(line.begin(), line.end(), u8"end_header")) {
                    end_header_read = true;
                    break;
                }
            }
        }

        if (end_header_read) {
            if (!property_x_read || !property_y_read || !property_z_read ||
                !property_s_read || !property_t_read ||
                !property_list_vertex_indices_read || !face_count_read || !vertex_count_read)
                throw std::runtime_error(u8"Invalid PLY file (missing something)");

            vertex_data.clear();
            element_data.clear();
            vertex_data.reserve(vertex_count * 5);
            element_data.reserve(face_count * 3);

            if (is_ascii) {
                return parse_ply_ascii(ply_input_stream,
                                       vertex_data,
                                       element_data,
                                       vertex_count,
                                       face_count,
                                       property_count,
                                       property_x_index,
                                       property_y_index,
                                       property_z_index,
                                       property_s_index,
                                       property_t_index);
            } else {
                return parse_ply_binary(ply_input_stream,
                                        vertex_data,
                                        element_data,
                                        vertex_count,
                                        face_count,
                                        property_list_vertex_indices_count_type,
                                        property_list_vertex_indices_index_type,
                                        property_count,
                                        property_x_index,
                                        property_y_index,
                                        property_z_index,
                                        property_s_index,
                                        property_t_index);
            }
        }

        return true;
    }
}
