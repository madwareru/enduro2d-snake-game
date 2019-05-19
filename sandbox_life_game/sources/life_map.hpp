#ifndef MAP_HPP
#define MAP_HPP
#include <array>
#include <enduro2d/enduro2d.hpp>

namespace map_definition {
    const e2d::u8 map_width  = 80;
    const e2d::u8 map_height = 45;
    const e2d::u8 tile_size  = 16;

    const e2d::u8 back_tile_1  = 0;
    const e2d::u8 back_tile_2  = 1;
    const e2d::u8 living_tile  = 2;
    const e2d::u8 blocker_tile = 3;

    const std::array<e2d::v2f,4> tile_uv_offsets = {
        e2d::v2f{0.0f, 0.5f},
        e2d::v2f{0.5f, 0.5f},
        e2d::v2f{0.0f, 0.0f},
        e2d::v2f{0.5f, 0.0f}
    };
}

#endif // MAP_HPP
