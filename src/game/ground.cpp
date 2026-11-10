// The player never walks into the ground: off the ground under a face of the world's blocks with none beneath him,
// inside the rock, he goes back to where he last stood on one. A plain bug fix, always on.
//
// The world's blocks hold faces, which are drawn and which the floor test stands the player on, and lines, the walls he
// walks into (each block's list at +0x18, 20 bytes a line, its count at +0x3E). A face steeper than he can climb is no
// floor, so where a rock wall rises from the ground it is the line along its foot that stops him, and where the
// prototype left a wall without one he walks onto its slope, the floor test finds nothing under him, and he drops
// through the inside of the rock into the void below the map, which puts him back at the last setup point. Golden
// Plains' block 399 (cell (1, 23)) has lines only around the hollow by the portal-spell door, none along the rock wall
// north of it: Krystal, walking west along its foot from (1082, 296, 14888), is held where the slope steepens at (958,
// 307, 14854), and turned along it she steps onto its faces at x 948, drops through and is put back at (1082, 340,
// 14888). The faces and lines are the ROM's, so the console would drop her too (not played there).
//
// dino_ground_keep_out, us.toml's hook after func_80046BC8 (each frame), looks at the loaded blocks' faces over the
// player's place. With one at or below his feet it keeps the place as where he last stood. With none there, one above
// his feet, and the floor test finding no floor under him (his floor type, dll210.70, is -1), he is inside the rock,
// and his place (+0xC and +0x18: x, y and z) goes back to the last, if that is within 60 units (no warp or cutscene
// jump). So a wall whose line is missing holds him as the line would, and nothing else changes: a step off a ledge
// into the open has no face over him, a step under an overhang has the ground beneath, and on an object's floor (a
// lift, a bridge) his floor type is not -1. Played: held at that wall for 30 s with the stick against it, where the
// same walk fell 9 s in. DINO_LOG_GROUND=1 prints each time he is put back.
//
// A block (the table at 0x800B96B4, by the slot each layer's grid of marks at 0x800B96D0 gives the cell; streaming.cpp
// says how the grid is laid) holds its vertices at +4 (16 bytes each: x, y and z as halfwords, x and z from its
// cell's corner, y the world's), its faces at +8 (8 bytes each), and its batches at +0xC (24 bytes each, the count at
// +0x36 and one more ending the list): a batch's +4 is its first vertex and +6 its first face. As the block loads,
// func_800493B4 packs each face's three vertices, from the batch's first, into its first word at bits 13, 7 and 1
// (5, 6 and 6 bits) and its normal above them. A batch flagged 0x20000000 holds its heights in another form
// (func_80048724 0x80048948) and is left out.

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <cstring>

#include "recomp.h"

namespace {
    constexpr int32_t player_object = 0x80023914;
    constexpr gpr grid_corner_x = (gpr)(int32_t)0x80092A68;
    constexpr gpr grid_corner_z = (gpr)(int32_t)0x80092A6C;
    constexpr gpr grid_marks = (gpr)(int32_t)0x800B96D0;
    constexpr gpr block_table = (gpr)(int32_t)0x800B96B4;
    constexpr int layers = 5;
    constexpr int block_count = 80;
    constexpr float cell_size = 640.0f;
    constexpr int floor_type_offset = 4 + 4 * 70;
    constexpr float farthest_back = 60.0f;

    struct Place {
        float local[3];
        float world[3];
    };

    // Where the player last stood with a face under him.
    struct {
        bool known = false;
        Place place;
    } safe;

    float real(uint8_t* rdram, gpr address) {
        int32_t bits = MEM_W(0, address);
        float value;
        memcpy(&value, &bits, 4);
        return value;
    }

    void set_real(uint8_t* rdram, gpr address, float value) {
        int32_t bits;
        memcpy(&bits, &value, 4);
        MEM_W(0, address) = bits;
    }

    gpr word(uint8_t* rdram, gpr address) {
        return (gpr)(int32_t)MEM_W(0, address);
    }

    Place place_of(uint8_t* rdram, gpr player) {
        Place place;
        for (int i = 0; i < 3; i++) {
            place.local[i] = real(rdram, player + 0xC + 4 * i);
            place.world[i] = real(rdram, player + 0x18 + 4 * i);
        }
        return place;
    }

    void set_place(uint8_t* rdram, gpr player, const Place& place) {
        for (int i = 0; i < 3; i++) {
            set_real(rdram, player + 0xC + 4 * i, place.local[i]);
            set_real(rdram, player + 0x18 + 4 * i, place.world[i]);
        }
    }

    // The height of the face (x1, z1)-(x2, z2)-(x3, z3) at (x, z), if (x, z) is over it.
    bool height_at(const float v[3][3], float x, float z, float* y) {
        float d = (v[1][2] - v[2][2]) * (v[0][0] - v[2][0]) + (v[2][0] - v[1][0]) * (v[0][2] - v[2][2]);
        if (fabsf(d) < 1e-6f) {
            return false;
        }
        float a = ((v[1][2] - v[2][2]) * (x - v[2][0]) + (v[2][0] - v[1][0]) * (z - v[2][2])) / d;
        float b = ((v[2][2] - v[0][2]) * (x - v[2][0]) + (v[0][0] - v[2][0]) * (z - v[2][2])) / d;
        float c = 1.0f - a - b;
        if (a < 0.0f || b < 0.0f || c < 0.0f) {
            return false;
        }
        *y = a * v[0][1] + b * v[1][1] + c * v[2][1];
        return true;
    }

    // Whether the loaded blocks have a face over (x, z) above y, and one at or below it.
    void faces_over(uint8_t* rdram, float x, float y, float z, bool* above, bool* below) {
        *above = false;
        *below = false;
        int cell_x = (int)floorf(x / cell_size), cell_z = (int)floorf(z / cell_size);
        int gx = cell_x - (int32_t)MEM_W(0, grid_corner_x), gz = cell_z - (int32_t)MEM_W(0, grid_corner_z);
        if (gx < 0 || gx >= 16 || gz < 0 || gz >= 16) {
            return;
        }
        float lx = x - cell_x * cell_size, lz = z - cell_z * cell_size;
        for (int layer = 0; layer < layers; layer++) {
            gpr marks = word(rdram, grid_marks + 4 * layer);
            if (marks == 0) {
                continue;
            }
            int slot = (int8_t)MEM_B(gz * 16 + gx, marks);
            if (slot < 0 || slot >= block_count) {
                continue;
            }
            gpr block = word(rdram, word(rdram, block_table) + 4 * slot);
            if (block == 0) {
                continue;
            }
            gpr vertices = word(rdram, block + 0x4), faces = word(rdram, block + 0x8);
            gpr batches = word(rdram, block + 0xC);
            int batch_count = (int16_t)MEM_H(0, block + 0x36);
            for (int b = 0; b < batch_count; b++) {
                gpr batch = batches + 24 * b;
                if (MEM_W(0, batch) & 0x20000000) {
                    continue;
                }
                int first_vertex = (int16_t)MEM_H(0, batch + 4);
                int first_face = (int16_t)MEM_H(0, batch + 6), end_face = (int16_t)MEM_H(0, batch + 24 + 6);
                for (int f = first_face; f < end_face; f++) {
                    uint32_t packed = (uint32_t)MEM_W(0, faces + 8 * f);
                    int index[3] = {
                        (int)((packed >> 13) & 0x1F), (int)((packed >> 7) & 0x3F), (int)((packed >> 1) & 0x3F)};
                    float v[3][3];
                    for (int i = 0; i < 3; i++) {
                        gpr vertex = vertices + 16 * (first_vertex + index[i]);
                        for (int axis = 0; axis < 3; axis++) {
                            v[i][axis] = (float)(int16_t)MEM_H(0, vertex + 2 * axis);
                        }
                    }
                    float face_y;
                    if (!height_at(v, lx, lz, &face_y)) {
                        continue;
                    }
                    if (face_y > y) {
                        *above = true;
                    }
                    else {
                        *below = true;
                    }
                }
            }
        }
    }
}

extern "C" void dino_ground_keep_out(uint8_t* rdram, recomp_context* ctx) {
    recomp_context saved = *ctx;
    ctx->r29 -= 0x40;
    LOOKUP_FUNC(player_object)(rdram, ctx);
    gpr player = ctx->r2;
    if (player == 0) {
        *ctx = saved;
        safe.known = false;
        return;
    }
    ctx->r4 = player;
    LOOKUP_FUNC(word(rdram, word(rdram, word(rdram, player + 0x68)) + floor_type_offset))(rdram, ctx);
    bool on_ground = (int32_t)ctx->r2 != -1;
    *ctx = saved;

    Place now = place_of(rdram, player);
    bool above, below;
    faces_over(rdram, now.world[0], now.world[1], now.world[2], &above, &below);
    if (below) {
        safe.known = true;
        safe.place = now;
        return;
    }
    if (on_ground || !above || !safe.known) {
        return;
    }
    float dx = now.world[0] - safe.place.world[0], dz = now.world[2] - safe.place.world[2];
    if (dx * dx + dz * dz >= farthest_back * farthest_back) {
        safe.known = false;
        return;
    }
    static const bool logging = std::getenv("DINO_LOG_GROUND") != nullptr;
    if (logging) {
        fprintf(stderr, "ground: in the ground at (%.0f, %.0f, %.0f), back to (%.0f, %.0f, %.0f)\n", now.world[0],
            now.world[1], now.world[2], safe.place.world[0], safe.place.world[1], safe.place.world[2]);
    }
    set_place(rdram, player, safe.place);
}
