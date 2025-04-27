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

#include "common_info.h"
#include "common_variable_8x16_sprite_font.h"

#include "bn_log_backend.h"
#include "bn_config_log.h"

#include "palette_cycler.h"


#define BN_CFG_LOG_ENABLED

constexpr uint8_t sewerMap [16*16] = {
    41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,
    41,2,2,2,2,2,2,2,2,2,2,2,2,2,2,41,
    41,2,1,1,1,1,1,1,1,1,1,1,1,1,2,41,
    41,2,1,3,3,3,3,3,1,3,3,3,3,1,2,41,
    41,2,1,80,3,3,3,3,3,3,3,3,3,1,2,41,
    41,2,1,80,80,80,80,80,80,80,3,3,3,1,2,41,
    41,2,1,80,80,80,80,80,80,80,80,80,3,1,2,41,
    41,2,1,56,56,56,80,80,80,4,4,80,80,1,2,41,
    41,2,1,56,56,56,80,80,80,4,4,4,4,1,2,41,
    41,2,1,56,56,56,56,80,80,4,4,4,4,1,2,41,
    41,2,1,56,56,56,56,56,80,4,4,4,4,1,2,41,
    41,2,1,56,56,56,56,56,56,56,56,56,56,1,2,41,
    41,2,1,1,1,1,1,1,1,1,1,1,13,1,2,41,
    41,2,1,1,1,1,1,1,1,1,1,1,1,1,2,41,
    41,2,2,2,2,2,2,2,2,2,2,2,2,2,2,41,
    41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41
};

// constexpr bn::color palette1_colors[16] = {
//     bn::color(0, 0, 0),     // Transparent/Black (often unused index 0)
//     bn::colors::red,
//     bn::colors::green,
//     bn::colors::white,
//     bn::colors::gray,
//     bn::colors::blue, // 5
//     bn::color(0,0,0), bn::color(0,0,0), bn::color(0,0,0), bn::color(0,0,0), // 6-9
//     bn::color(0,0,0), bn::color(0,0,0), bn::color(0,0,0), bn::color(0,0,0), // 10-13
//     bn::color(0,0,0), bn::color(0,0,0) // 14-15
// };

// const bn::tile makeTile(uint32_t pattern)
// {
//     bn::tile tile;
//     for(int i = 0; i < 8; ++i)
//     {
//         tile.data[i] = pattern;
//     }
//     return tile;
// }

//constexpr bn::bg_palette_item palette1_item(palette1_colors, bn::bpp_mode::BPP_4);

// Tile Set 1 (4bpp) - For Layer 0
// Each bn::tile represents 8x8 pixels. For 4bpp, 32 bytes are needed per tile.
// bn::tile is 64 bytes, but Butano handles the bpp setting correctly.
// Lower nibble = left pixel, Upper nibble = right pixel in a byte.
// bn::tile tileset1_tiles[] = {
//     // Tile 0: Often fully transparent or unused
//     makeTile(0x11111111),
//     makeTile(0x11111111),
// };

// Map Data 1 (for Layer 0) - Uses tileset1 and palette1
// Dimensions: 32x32 tiles (minimum size for regular background)
// Each cell specifies (tile_index, palette_id)
// We will use palette ID 0 for this layer.
constexpr int map1_width = 32;
constexpr int map1_height = 32;
bn::regular_bg_map_cell map1_cells[map1_width * map1_height] = {
    // Fill with a pattern - e.g., Tile 1 (Solid Red) border, Tile 2 inside
    // NOTE: This is tedious manually! grit is highly recommended.
    // Using a loop in code is better for simple patterns:
    // We'll initialize it dynamically below for brevity here.
};

//constexpr bn::regular_bg_tiles_item tileset1_item(tileset1_tiles, bn::bpp_mode::BPP_4);

int tileForMetaMapCell(int metaTileNumber, int subTileIndex)
{
    // Example logic to determine tile index based on cell and sub-tile index
    // This is a placeholder; actual logic will depend on your map design
    int x = metaTileNumber % 20;
    int y = metaTileNumber / 20;

    int baseTileIndex = (y * 40 * 2) + (x *2); // Base index for the tile
    if (subTileIndex == 1) {
        return ++baseTileIndex;
    } else if (subTileIndex == 2) {
        return baseTileIndex + 40; // Checkerboard tile
    } else if (subTileIndex == 3) {
        return baseTileIndex + 41;
    } 

    return baseTileIndex;
}

int main(){
    bn::core::init();
    //bn::bg_palette_ptr bg_palette_0 = palette1_item.create_palette();
    //bn::regular_bg_tiles_ptr bg_tiles_0 = tileset1_item.create_tiles();
    

    //bn::vector<bn::regular_bg_map_cell, map1_width * map1_height> map1_cells_dynamic;
    //constexpr int pal_id_0 = 0; // Palette ID for layer 0

    constexpr int sewer_map_width = 16;
    constexpr int sewer_map_height = 16;
    constexpr int sewer_map_cell_count = sewer_map_width * sewer_map_height;
     
    for(int i = 0; i < sewer_map_cell_count; ++i) {
        uint8_t metaTileNumber = sewerMap[i];
        uint8_t mapCellX = i % sewer_map_width;
        uint8_t mapCellY = i / sewer_map_height; 

        for(int j = 0; j < 4; ++j) {
            uint8_t subTileX = mapCellX * 2 + (j % 2);
            uint8_t subTileY = mapCellY * 2 + (j / 2);
            map1_cells[subTileX + (subTileY*32)] = tileForMetaMapCell(metaTileNumber, j);
        }

       // map1_cells[i] = bn::regular_bg_map_cell(0); // Initialize with transparent tile (index 0)
    }

    // for(int y = 0; y < map1_height; ++y) {
    //     for(int x = 0; x < map1_width; ++x) {
    //         int tile_index = 0;
    //         if (y == 0 || y == map1_height - 1 || x == 0 || x == map1_width - 1) {
    //             tile_index = 1; // Border tile (Solid Red)
    //         } else if ((x + y) % 2 == 0) {
    //              tile_index = 2; // Checkerboard tile
    //         } else {
    //              tile_index = 3; // Lines tile
    //         }
            

    //         map1_cells[y*map1_width + x]= 2;//(bn::regular_bg_map_cell((x*y)%640));
    //     }
    // }

    bn::regular_bg_map_item map1_item(map1_cells[0], bn::size(map1_width, map1_height));
    //bn::regular_bg_map_ptr bg_map_0 = map1_item.create_map(bg_tiles_0, bg_palette_0);

     bn::regular_bg_ptr bg_layer_0 = bn::regular_bg_item(
        bn::regular_bg_tiles_items::sewers_16, // Use optional creation from item
        bn::regular_bg_tiles_items::sewers_16_palette,
            map1_item) // Link map to specific tiles/palette
        .create_bg(0, 0); // Place BG at screen origin (0,0)

    // Set properties (optional, but good practice)
    bg_layer_0.set_priority(3); // Lower numbers are drawn behind higher numbers (3 is lowest priority)
    bg_layer_0.set_visible(true);

    
    PaletteCycler palette_cycler(
        bg_layer_0.palette(), // Create palette from item
        0, // Start index for cycling (0-15)
        16, // Number of colors to cycle (1-16)
        5); // Delay frames between cycles

    while(1)
    {
        // Update the screen
            bn::core::update();
       // palette_cycler.update(); // Update the palette cycling
    }
}


// namespace
// {
//     void camera_scene(bn::camera_ptr& camera, bn::sprite_text_generator& text_generator)
//     {
//         constexpr bn::string_view info_text_lines[] = {
//             "PAD: move camera",
//             "",
//             "START: go to next scene",
//         };

//         common::info info("Camera position", info_text_lines, text_generator);

//         while(! bn::keypad::start_pressed())
//         {
//             if(bn::keypad::left_held())
//             {
//                 camera.set_x(camera.x() - 1);
//             }
//             else if(bn::keypad::right_held())
//             {
//                 camera.set_x(camera.x() + 1);
//             }

//             if(bn::keypad::up_held())
//             {
//                 camera.set_y(camera.y() - 1);
//             }
//             else if(bn::keypad::down_held())
//             {
//                 camera.set_y(camera.y() + 1);
//             }

//             info.update();
//             bn::core::update();
//         }

//         camera.set_position(0, 0);
//     }

//     void camera_actions_scene(bn::camera_ptr& camera, bn::sprite_text_generator& text_generator)
//     {
//         constexpr bn::string_view info_text_lines[] = {
//             "START: go to next scene",
//         };

//         common::info info("Camera actions", info_text_lines, text_generator);

//         bn::fixed amplitude = 30;
//         camera.set_position(-amplitude, -amplitude);

//         bn::camera_move_loop_action camera_action(camera, 120, amplitude, amplitude);

//         while(! bn::keypad::start_pressed())
//         {
//             camera_action.update();
//             info.update();
//             bn::core::update();
//         }

//         camera.set_position(0, 0);
//     }
// }

// struct tile_map{
//     static constexpr int columns = 32;
//     static constexpr int rows = 32;
//     static constexpr int cells_count = columns * rows;

//     alignas(int) bn::regular_bg_map_cell cells[cells_count];
//     bn::regular_bg_map_item map_item;

//     tile_map() :
//         map_item(cells[0], bn::size(tile_map::columns, tile_map::rows))
//     {
//         reset();
//     }

//     void reset()
//     {
//         for(int cell_index : bn::range(cells_count))
//         {
//             cells[cell_index] = bn::regular_bg_map_cell(0);
//         }
//     }
// };

// int main()
// {
//     bn::core::init();

//     BN_LOG("bn::core::init() done");

//     bn::regular_bg_tiles_item sewer_tiles = bn::regular_bg_tiles_items::sewers_16;
//     bn::bg_palette_item sewer_palette = bn::regular_bg_tiles_items::sewers_16_palette;
//     bn::regular_bg_map_cell sewers_bn_gfxMap1[1024];
//     bn::regular_bg_map_item sewer_map = bn::regular_bg_map_item(
//         sewers_bn_gfxMap1[0],
//         bn::size(32, 32),
//         bn::compression_type::NONE,
//         1,
//         false);


//     bn::sprite_text_generator text_generator(common::variable_8x16_sprite_font);

//     bn::regular_bg_ptr land_bg = bn::regular_bg_items::sewers.create_bg(0, 0);

//     //bn::regular_bg_ptr clouds_bg = bn::regular_bg_items::clouds.create_bg(0, 0);
//    // bn::blending::set_transparency_alpha(0.5);
//     //clouds_bg.set_blending_enabled(true);

//     bn::sprite_ptr dino_sprite = bn::sprite_items::dino.create_sprite(-60, 0);

//     bn::window outside_window = bn::window::outside();
//     //outside_window.set_show_bg(clouds_bg, false);

//     bn::rect_window internal_window = bn::rect_window::internal();
//     internal_window.set_boundaries(-48, 8, 48, 104);

//     bn::camera_ptr camera = bn::camera_ptr::create(0, 0);
//     land_bg.set_camera(camera);
//     dino_sprite.set_camera(camera);
//     internal_window.set_camera(camera);

//     while(true)
//     {
//         camera_scene(camera, text_generator);
//         bn::core::update();

//         camera_actions_scene(camera, text_generator);
//         bn::core::update();
//     }
// }
