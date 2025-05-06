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
// #include "bn_regular_bg_items_sewers.h"
#include "bn_regular_bg_tiles_items_sewers_16.h"
#include "bn_regular_bg_tiles_items_water0.h"

#include "bn_sprite_items_warrior3.h"
#include "bn_sprite_actions.h"
#include "bn_sprite_builder.h"
#include "bn_sprites_actions.h"
#include "bn_sprite_animate_actions.h"

#include "common_info.h"
#include "common_variable_8x16_sprite_font.h"

#include "bn_log_backend.h"
#include "bn_config_log.h"

#include "palette_cycler.h"

// #define BN_CFG_LOG_ENABLED

constexpr uint8_t sewerMap[16 * 16] = {
    86,1,93,0,0,0,0,0,0,0,0,0,0,0,0,0,
86,73,58,58,58,58,93,0,0,0,0,0,0,0,0,0,
86,1,1,1,1,1,62,0,0,0,0,0,0,0,0,0,
86,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,
86,1,12,1,1,1,93,0,0,0,0,0,0,0,0,0,
85,1,1,1,1,1,90,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

constexpr uint8_t water[16 * 16] = {
    1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2,
    3, 4, 3, 4, 3, 4, 3, 4, 3, 4, 3, 4, 3, 4, 3, 4,
    1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2,
    3, 4, 3, 4, 3, 4, 3, 4, 3, 4, 3, 4, 3, 4, 3, 4,
    1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2,
    3, 4, 3, 4, 3, 4, 3, 4, 3, 4, 3, 4, 3, 4, 3, 4,
    1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2,
    3, 4, 3, 4, 3, 4, 3, 4, 3, 4, 3, 4, 3, 4, 3, 4,
    1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2,
    3, 4, 3, 4, 3, 4, 3, 4, 3, 4, 3, 4, 3, 4, 3, 4,
    1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2,
    3, 4, 3, 4, 3, 4, 3, 4, 3, 4, 3, 4, 3, 4, 3, 4,
    1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2,
    3, 4, 3, 4, 3, 4, 3, 4, 3, 4, 3, 4, 3, 4, 3, 4,
    1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2,
    3, 4, 3, 4, 3, 4, 3, 4, 3, 4, 3, 4, 3, 4, 3, 4};

constexpr int native_tile_width = 32;
constexpr int native_tile_height = 32;
constexpr int sewer_map_width = 16;
constexpr int sewer_map_height = 16;

bn::regular_bg_map_cell water_cells_water[native_tile_width * native_tile_height];
constexpr int sewer_tilesheet_width = 20;

int tileForMetaMapCell(int metaTileNumber, int subTileIndex, int tileSheetWidth)
{
    int x = metaTileNumber % tileSheetWidth;
    int y = metaTileNumber / tileSheetWidth;

    int nativeTileSheetWidth = tileSheetWidth * 2;

    int baseTileIndex = (y * nativeTileSheetWidth * 2) + (x * 2); // Base index for the tile
    if (subTileIndex == 1)
    {
        return ++baseTileIndex;
    }
    else if (subTileIndex == 2)
    {
        return baseTileIndex + nativeTileSheetWidth; // Checkerboard tile
    }
    else if (subTileIndex == 3)
    {
        return baseTileIndex + nativeTileSheetWidth + 1;
    }

    return baseTileIndex;
}

void loadTilesFromMetaMap(int metaMapWidth, int metaMapHeight, int nativeTileSheetWidth, uint16_t* tile_cell_map, const uint8_t* meta_tile_map){
    for (int i = 0; i < metaMapWidth * metaMapHeight; ++i)
    {
        uint8_t metaTileNumber = meta_tile_map[i];
        uint8_t mapCellX = i % metaMapWidth;
        uint8_t mapCellY = i / metaMapHeight;

        for (int j = 0; j < 4; ++j)
        {
            uint8_t subTileX = mapCellX * 2 + (j % 2);
            uint8_t subTileY = mapCellY * 2 + (j / 2);
            tile_cell_map[subTileX + (subTileY * 32)] = tileForMetaMapCell(metaTileNumber, j, nativeTileSheetWidth);
        }
    }
}

int main()
{
    bn::core::init();

    bn::regular_bg_map_cell sewers_cells[native_tile_width * native_tile_height];
    loadTilesFromMetaMap(sewer_map_width, sewer_map_height, sewer_tilesheet_width, sewers_cells, sewerMap);
    bn::regular_bg_map_item sewer_tile_cells(sewers_cells[0], bn::size(native_tile_width, native_tile_height));

    bn::regular_bg_ptr bg_layer_0 = bn::regular_bg_item(
                                        bn::regular_bg_tiles_items::sewers_16, // Use optional creation from item
                                        bn::regular_bg_tiles_items::sewers_16_palette,
                                        sewer_tile_cells)        // Link map to specific tiles/palette
                                        .create_bg(0, 0); // Place BG at screen origin (0,0)

    bg_layer_0.set_priority(2); // Lower numbers are drawn behind higher numbers (3 is lowest priority)
    bg_layer_0.set_visible(true);


    bn::regular_bg_map_cell water_cells[native_tile_width * native_tile_height];
    loadTilesFromMetaMap(sewer_map_width, sewer_map_height, 5, water_cells, water);
    bn::regular_bg_map_item water_tile_cells(water_cells[0], bn::size(native_tile_width, native_tile_height));

    bn::regular_bg_ptr bg_layer_1 = bn::regular_bg_item(
                                        bn::regular_bg_tiles_items::water0, // Use optional creation from item
                                        bn::regular_bg_tiles_items::water0_palette,
                                        water_tile_cells)        // Link map to specific tiles/palette
                                        .create_bg(0, 0); // Place BG at screen origin (0,0)

    bg_layer_1.set_priority(3); // Lower numbers are drawn behind higher numbers (3 is lowest priority)
    bg_layer_1.set_visible(true);    

    bn::sprite_ptr warrior_sprite = bn::sprite_items::warrior3.create_sprite(0, 0);
    bn::sprite_animate_action<7> action = bn::create_sprite_animate_action_forever(
        warrior_sprite, 16, bn::sprite_items::warrior3.tiles_item(), 0, 1, 2, 3, 4, 5, 6);

    warrior_sprite.set_horizontal_flip(true); // Flip the sprite horizontally
    warrior_sprite.set_bg_priority(2);        // Set sprite priority to be drawn above the background

    bn::camera_ptr camera = bn::camera_ptr::create(8, 8);
    bg_layer_0.set_camera(camera);
    bg_layer_1.set_camera(camera); // Link camera to the background

    PaletteCycler palette_cycler(
        bg_layer_1.palette(), // Create palette from item
        2,                    // Start index for cycling (0-15)
        2,                   // Number of colors to cycle (1-16)
        15);                   // Delay frames between cycles

    while (1)
    {
        action.update();
        bn::core::update();
        if (bn::keypad::left_pressed())
        {
            camera.set_x(camera.x() - 16);
            warrior_sprite.set_horizontal_flip(true);
        }
        else if (bn::keypad::right_pressed())
        {
            camera.set_x(camera.x() + 16);
            warrior_sprite.set_horizontal_flip(false);
        }

        if (bn::keypad::up_pressed())
        {
            camera.set_y(camera.y() - 16);
        }
        else if (bn::keypad::down_pressed())
        {
            camera.set_y(camera.y() + 16);
        }
        
        bg_layer_1.set_top_left_y(bg_layer_1.top_left_y() + .01); // Move the water layer down
        palette_cycler.update(); // Update the palette cycling
    }
}
