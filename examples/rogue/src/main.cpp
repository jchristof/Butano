/*
 * Copyright (c) 2020-2025 Gustavo Valiente gustavo.valiente@protonmail.com
 * zlib License, see LICENSE file.
 */

#include "bn_core.h"
#include "bn_keypad.h"
#include "bn_blending.h"
#include "bn_rect_window.h"
#include "bn_bg_palette_ptr.h"
#include "bn_regular_bg_ptr.h"
#include "bn_camera_actions.h"
#include "bn_regular_bg_tiles_ptr.h"
#include "bn_regular_bg_map_ptr.h"
#include "bn_sprite_text_generator.h"
#include "bn_log.h"
#include "bn_colors.h"
#include "bn_memory.h"

#include "bn_sprite_items_dino.h"
#include "bn_regular_bg_items_land.h"
#include "bn_regular_bg_items_clouds.h"
#include "bn_regular_bg_items_sewers.h"
#include "bn_regular_bg_tiles_items_sewers_16.h"

#include "bn_sprite_items_warrior3.h"

#include "common_info.h"
#include "common_variable_8x16_sprite_font.h"

#include "bn_log_backend.h"
#include "bn_config_log.h"

#include "palette_cycler.h"

#define BN_CFG_LOG_ENABLED

constexpr uint8_t sewerMap[16 * 16] = {
    41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41,
    41, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 41,
    41, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 41,
    41, 2, 1, 3, 3, 3, 3, 3, 1, 3, 3, 3, 3, 1, 2, 41,
    41, 2, 1, 80, 3, 3, 3, 3, 3, 3, 3, 3, 3, 1, 2, 41,
    41, 2, 1, 80, 80, 80, 80, 80, 80, 80, 3, 3, 3, 1, 2, 41,
    41, 2, 1, 80, 80, 80, 80, 80, 80, 80, 80, 80, 3, 1, 2, 41,
    41, 2, 1, 56, 56, 56, 80, 80, 80, 4, 4, 80, 80, 1, 2, 41,
    41, 2, 1, 56, 56, 56, 80, 80, 80, 4, 4, 4, 4, 1, 2, 41,
    41, 2, 1, 56, 56, 56, 56, 80, 80, 4, 4, 4, 4, 1, 2, 41,
    41, 2, 1, 56, 56, 56, 56, 56, 80, 4, 4, 4, 4, 1, 2, 41,
    41, 2, 1, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 1, 2, 41,
    41, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 13, 1, 2, 41,
    41, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 41,
    41, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 41,
    41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41};

constexpr int map1_width = 32;
constexpr int map1_height = 32;
bn::regular_bg_map_cell map1_cells[map1_width * map1_height];

namespace
{
    void camera_scene(bn::camera_ptr &camera)
    {
        while (!bn::keypad::start_pressed())
        {
            if (bn::keypad::left_held())
            {
                camera.set_x(camera.x() - 1);
            }
            else if (bn::keypad::right_held())
            {
                camera.set_x(camera.x() + 1);
            }

            if (bn::keypad::up_held())
            {
                camera.set_y(camera.y() - 1);
            }
            else if (bn::keypad::down_held())
            {
                camera.set_y(camera.y() + 1);
            }

            bn::core::update();
        }

        camera.set_position(0, 0);
    }

}

int tileForMetaMapCell(int metaTileNumber, int subTileIndex)
{
    int x = metaTileNumber % 20;
    int y = metaTileNumber / 20;

    int baseTileIndex = (y * 40 * 2) + (x * 2); // Base index for the tile
    if (subTileIndex == 1)
    {
        return ++baseTileIndex;
    }
    else if (subTileIndex == 2)
    {
        return baseTileIndex + 40; // Checkerboard tile
    }
    else if (subTileIndex == 3)
    {
        return baseTileIndex + 41;
    }

    return baseTileIndex;
}

int main()
{
    bn::core::init();

    constexpr int sewer_map_width = 16;
    constexpr int sewer_map_height = 16;
    constexpr int sewer_map_cell_count = sewer_map_width * sewer_map_height;

    for (int i = 0; i < sewer_map_cell_count; ++i)
    {
        uint8_t metaTileNumber = sewerMap[i];
        uint8_t mapCellX = i % sewer_map_width;
        uint8_t mapCellY = i / sewer_map_height;

        for (int j = 0; j < 4; ++j)
        {
            uint8_t subTileX = mapCellX * 2 + (j % 2);
            uint8_t subTileY = mapCellY * 2 + (j / 2);
            map1_cells[subTileX + (subTileY * 32)] = tileForMetaMapCell(metaTileNumber, j);
        }
    }

    bn::regular_bg_map_item map1_item(map1_cells[0], bn::size(map1_width, map1_height));

    bn::regular_bg_ptr bg_layer_0 = bn::regular_bg_item(
                                        bn::regular_bg_tiles_items::sewers_16, // Use optional creation from item
                                        bn::regular_bg_tiles_items::sewers_16_palette,
                                        map1_item)        // Link map to specific tiles/palette
                                        .create_bg(0, 0); // Place BG at screen origin (0,0)

    // Set properties (optional, but good practice)
    bg_layer_0.set_priority(3); // Lower numbers are drawn behind higher numbers (3 is lowest priority)
    bg_layer_0.set_visible(true);

    bn::sprite_ptr warrior_sprite = bn::sprite_items::warrior3.create_sprite(-60, 0);

    bn::camera_ptr camera = bn::camera_ptr::create(0, 0);
    bg_layer_0.set_camera(camera); // Link camera to the background

    PaletteCycler palette_cycler(
        bg_layer_0.palette(), // Create palette from item
        0,                    // Start index for cycling (0-15)
        16,                   // Number of colors to cycle (1-16)
        5);                   // Delay frames between cycles

    while (1)
    {
        bn::core::update();
        camera_scene(camera);
        palette_cycler.update(); // Update the palette cycling
    }
}
