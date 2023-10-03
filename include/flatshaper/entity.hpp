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

#ifndef FLATSHAPER_ENTITY_HPP
#define FLATSHAPER_ENTITY_HPP

#include <cinttypes>

typedef std::uint64_t entityid_t;

namespace flatshaper {
    entityid_t generate_entity_id();
    void delete_entity(entityid_t entityid);
    bool is_entity_valid(entityid_t entityid);
}

#endif
