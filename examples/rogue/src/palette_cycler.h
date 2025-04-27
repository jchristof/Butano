#ifndef PALETTE_CYCLER_H
#define PALETTE_CYCLER_H

#include "bn_bg_palette_ptr.h" // Or bn_sprite_palette_ptr
#include "bn_array.h"          // Use bn::array
#include "bn_span.h"           // Use bn::span for set_colors
#include "bn_color.h"
#include "bn_assert.h"

// Simple class to manage palette cycling animation for a 4bpp BG palette (max 16 colors)
class PaletteCycler
{
public:
    // Constructor:
    // palette: The 4bpp palette to modify.
    // start_index: The first color index in the palette range to cycle (0-15).
    // count: How many colors are in the cycling range.
    // delay_frames: How many game frames to wait before each step of the cycle.
    PaletteCycler(bn::bg_palette_ptr palette, int start_index, int count, int delay_frames) :
        _palette(palette),
        _start_index(start_index),
        _count(count),
        _delay_frames(delay_frames),
        _timer(0),
        _cycle_offset(0)
    {
        // --- Get Palette Size ---
        int palette_colors_count = _palette.colors_count(); // Get actual size

        // --- Input Validation ---
        // It's good practice to ensure the palette is indeed 4bpp if the class assumes it
        BN_ASSERT(palette.bpp() == bn::bpp_mode::BPP_4, "PaletteCycler currently only supports 4bpp palettes");
        BN_ASSERT(palette_colors_count > 0 && palette_colors_count <= 16, "PaletteCycler (array version) requires palette size between 1 and 16 (4bpp). Actual size: ", palette_colors_count);
        BN_ASSERT(start_index >= 0, "PaletteCycler: start_index cannot be negative");
        BN_ASSERT(count > 0, "PaletteCycler: count must be positive");
        BN_ASSERT(start_index + count <= palette_colors_count,
                  "PaletteCycler: range [", start_index, ", ", start_index + count, ") exceeds palette size ", palette_colors_count);
        BN_ASSERT(delay_frames >= 1, "PaletteCycler: delay_frames must be at least 1");
        BN_ASSERT(_original_cycle_colors.size() == 16, "PaletteCycler: _original_cycle_colors size is less than count");

        // Store the original colors in the cycling range
        for(int i = 0; i < palette_colors_count; ++i) {
             _temp_palette_colors.at(i) = _palette.colors().at(i);
        }

        // Store only the original colors from the specific range we will cycle
        for(int i = 0; i < _count; ++i) {
            // Use .at() for safety, accessing the temp buffer we just filled
            _original_cycle_colors.at(i) = _temp_palette_colors.at(start_index + i);
        }
    }

    // Call this once per frame in your main loop
    void update()
    {
        // No cycling needed if count is 1 (or less, though asserted > 0)
        if (_count <= 1)
        {
            return;
        }

        _timer++;
        if (_timer >= _delay_frames)
        {
            _timer = 0;

            // Increment the offset, wrapping around based on the count
            _cycle_offset = (_cycle_offset + 1) % _count;

            // Apply the rotation to the relevant part of the temporary buffer
            for (int i = 0; i < _count; ++i)
            {
                // Calculate which original color should go into the current slot 'i'
                // The original colors are stored compactly in _original_cycle_colors[0..._count-1]
                int original_color_index = (i + _cycle_offset) % _count;
                _temp_palette_colors.at(_start_index + i) = _original_cycle_colors.at(original_color_index);
            }

            // // Update the actual hardware palette RAM with the modified buffer
            // // Create a span of the correct size from the temporary array.
            // // The span points to the beginning of our array data, but its size
            // // matches the actual size of the hardware palette being managed.
            // bn::span<const bn::color> updated_colors_span(
            //     _temp_palette_colors.data(), // Pointer to the start of array data
            //     _palette.colors_count());    // The actual number of colors in the target palette

            for(int i = 0; i < _count; ++i) {
                // Update the palette with the new colors
                // The span is created to only cover the range we want to update
                _palette.set_color(i, _temp_palette_colors[i]);
               // updated_colors_span = bn::span<const bn::color>(updated_colors_span.data() + _start_index, _count);
            }
            
        }
    }

    // Optional: Allow changing the speed dynamically
    void set_delay_frames(int delay_frames) {
        //BN_ASSERT(delay_frames >= 1, "PaletteCycler: delay_frames must be at least 1");
        _delay_frames = delay_frames;
        _timer = 0; // Reset timer to avoid weird jumps
    }

private:
    bn::bg_palette_ptr _palette;
    int _start_index;
    int _count; // Number of colors in the cycle range
    int _delay_frames;
    int _timer;
    int _cycle_offset; // Tracks the current rotation step

    // Store the original colors in the cycling range using bn::array
    // Array is fixed size 16, but we only use the first '_count' elements.
    bn::array<bn::color, 16> _original_cycle_colors;

    // A temporary buffer holding the *entire* potential 16-color palette state using bn::array
    bn::array<bn::color, 16> _temp_palette_colors;
};

#endif // PALETTE_CYCLER_H