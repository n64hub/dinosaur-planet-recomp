// What RT64 needs to interpolate the game to the display's rate. us.toml's hooks call these with the game's display
// list pointers and each writes RT64's extended commands (rt64_extended_gbi.h) into the list, where the game is about to
// write its own. A console's microcode never sees them. At the framerate option's Original nothing is drawn between
// frames, so the groups change nothing.
//
// The game loads its camera as one matrix, projection times view, into the projection (func_800026C8, at 0x800028B0),
// and each frame's cameras take a group of their own: an untagged camera is always paired with the one before, which
// slides from shot to shot at every cut, and a tagged one only with the same id. A cut gets a new id, and a frame cut in
// also puts the world in a group of its own, or what it draws would still pair with the frame before: the world's origin
// follows the player (below), so a cut that moves it has every matrix of the world sliding by the difference. Everything
// is back to pairing a frame later.
//
// The world's origin, 0x80092A70 and 0x80092A74, is a multiple of a map cell's 640 units near the player, and it steps
// by a cell as the player walks, a step back at a cell's edge too. The view and every matrix of the world step with it,
// the two cancelling. So a step isn't a cut: RT64 pairs what the world draws by its place on the screen, which the step
// doesn't change, and RT64_INTERPOLATE_CAMERA_EYE places the camera's eye on the line between its two places, where
// the eye's step takes away the world's exactly. RT64 left to itself doesn't interpolate a translation that grows
// tenfold in a frame (rt64_rigid_body.cpp), which the step's 640 units always does, so the world would draw where it
// is while the eye is halfway: everything 320 units off, and above CloudRunner Fortress's clouds the eye outside its sky,
// a frame of the background colour alone. A frame whose origin steps puts the world and each object's model in a group
// that interpolates their translations.
//
// Camera 1, which draws an object's shadow into a texture, follows the origin with its raw position, so for it a step
// is a cut: that texture isn't blended for a frame, which doesn't show.
//
// Each object's model (func_80035000) is a group by the object, its model and the last cut, so each joint pairs in
// order with its own and two alike objects can't swap. The model's vertices, which the skinning writes anew each
// frame, are interpolated with it. An object that jumps gets a new group too. What the world draws by itself (its map
// blocks, sky, particles) has no group and RT64 pairs it, as it does for any game.
//
// What the commands cost: 24 bytes of the 36,000-byte list per object, 16 per camera and 16 for a step. The fullest list measured in
// the levels of eight areas was 11,200 bytes and a title screen's, which draws no objects, 29,800.

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unordered_map>

#include "recomp.h"

namespace {
    constexpr uint32_t extended_opcode = 0x64;                        // RT64_EXTENDED_OPCODE
    constexpr uint32_t hook_opcode = 0xE0;                            // RT64_HOOK_OPCODE under F3DEX2
    constexpr uint32_t hook_magic = 0x525464;                         // RT64_HOOK_MAGIC_NUMBER
    constexpr uint32_t hook_enable = 0x1;                             // RT64_HOOK_OP_ENABLE
    constexpr uint32_t op_matrix_group = (extended_opcode << 24) | 0x00000C; // G_EX_MATRIXGROUP_V1
    constexpr uint32_t op_pop_group = (extended_opcode << 24) | 0x00000D;    // G_EX_POPMATRIXGROUP_V1
    constexpr uint32_t component_skip = 0, component_interpolate = 1, component_auto = 2;
    constexpr uint32_t order_linear = 0, order_auto = 1;
    constexpr uint32_t mode_simple = 0, mode_decompose = 1;
    constexpr uint32_t aspect_auto = 2;

    // The cameras, 0x60 bytes each, up to the current one's index at 0x800A66FC: their position at +0x44, as func_800026C8
    // reads it to build the view, and their yaw, pitch and roll at +0x50.
    constexpr uint32_t cameras_address = 0x800A6278;
    constexpr uint32_t camera_size = 0x60;
    constexpr uint32_t camera_count = 12;

    constexpr uint32_t origin_address = 0x80092A70;
    constexpr uint32_t id_auto = 0xFFFFFFFF;                          // G_EX_ID_AUTO: paired by RT64, as with no group.

    // Measured over eight areas and the opening: the camera steps 0-9 units and 0-340 of 65,536 of a turn a game frame,
    // and a cut 33 units or more, with 2,000 of a turn or more whenever it is under 100 units.
    constexpr float jump_distance = 64.0f;
    constexpr float jump_turn = 2048.0f;

    void write(uint8_t* rdram, gpr* gfx, uint32_t w0, uint32_t w1) {
        MEM_W(0, *gfx) = (int32_t)w0;
        MEM_W(4, *gfx) = (int32_t)w1;
        *gfx += 8;
    }

    void write_group(uint8_t* rdram, gpr* gfx, uint32_t id, uint32_t mode, bool push, bool proj, uint32_t components, uint32_t vert, uint32_t order) {
        write(rdram, gfx, op_matrix_group, id);
        write(rdram, gfx, (push ? 1 : 0) | ((proj ? 1 : 0) << 1) | (mode << 2) | (components << 3) | (components << 5) | (components << 7) | (components << 9) |
            (components << 11) | (vert << 13) | (component_auto << 15) | (order << 17) | (aspect_auto << 20) | (component_skip << 22) | (component_auto << 24), 0);
    }

    float read_float(uint8_t* rdram, gpr address) {
        int32_t bits = MEM_W(0, address);
        float value;
        memcpy(&value, &bits, sizeof(value));
        return value;
    }

    struct Position {
        float x, y, z;
    };

    Position read_position(uint8_t* rdram, gpr address) {
        return { read_float(rdram, address), read_float(rdram, address + 4), read_float(rdram, address + 8) };
    }

    float distance(const Position& a, const Position& b) {
        float x = a.x - b.x, y = a.y - b.y, z = a.z - b.z;
        return sqrtf(x * x + y * y + z * z);
    }

    // How far something has been stepping, each game frame.
    struct Pace {
        float steady = 0.0f;
        float last = 0.0f;

        // A jump is past the limit, more than 4 times the steady step and unlike the step before it, so a camera that
        // sweeps fast from a stop is one jump and then a steady step, and two cuts running are both jumps.
        bool jumped(float step, float limit) {
            bool steps = (step <= limit) || (step <= steady * 4.0f) || ((step <= last * 2.0f) && (step * 2.0f >= last));
            if (steps) {
                steady = step;
            }

            last = step;
            return !steps;
        }
    };

    int turned(int from, int to) {
        return std::abs((int16_t)(to - from));
    }

    struct Camera {
        Position position = {};
        int yaw = 0, pitch = 0, roll = 0;
        Pace move, turn;
        uint32_t frame = 0;
        uint32_t cuts = 0;
    };

    struct Object {
        Position position = {};
        Pace move;
        uint32_t frame = 0;
        uint32_t jumps = 0;
    };

    Camera cameras[camera_count];
    std::unordered_map<uint32_t, Object> objects;
    uint32_t frame_number = 0;
    uint32_t world_cuts = 0;
    uint32_t origin_x = 0, origin_z = 0;
    bool origin_stepped = false;
    bool logging = getenv("DINO_LOG_CUTS") != nullptr;            // Prints each cut and jump, to tune the limits above.

    uint32_t object_id(uint32_t object, uint32_t model, uint32_t jumps) {
        uint32_t id = object * 0x9E3779B1u;
        id = (id ^ model) * 0x85EBCA6Bu;
        id = (id ^ jumps) * 0xC2B2AE35u;
        id = (id ^ world_cuts) * 0x27D4EB2Fu;
        return (id >> 1) | 1;                                         // Neither G_EX_ID_IGNORE nor G_EX_ID_AUTO.
    }
}

// At the start of func_8004225C, which draws the frame: a0 is the address of the list's pointer.
extern "C" void dino_interp_frame(uint8_t* rdram, recomp_context* ctx) {
    gpr gfx = (gpr)(int32_t)MEM_W(0, ctx->r4);
    write(rdram, &gfx, (hook_opcode << 24) | hook_magic, (hook_enable << 28) | extended_opcode);
    gpr origin = (gpr)(int32_t)origin_address;
    uint32_t x = (uint32_t)MEM_W(0, origin), z = (uint32_t)MEM_W(4, origin);
    origin_stepped = (x != origin_x) || (z != origin_z);
    origin_x = x;
    origin_z = z;
    if (origin_stepped) {
        write_group(rdram, &gfx, id_auto, mode_decompose, false, false, component_interpolate, component_skip, order_auto);
        if (logging) {
            fprintf(stderr, "step: frame %u\n", frame_number);
        }
    }

    MEM_W(0, ctx->r4) = (int32_t)gfx;
    frame_number++;
}

// Before func_800026C8 loads the camera: a1 is the address of the list's pointer, s0 the camera. A frame loads the
// main camera (0) twice and, while an object casts a shadow (func_8005BCE0 draws it into a texture), camera 1 between
// the two. A camera is judged at the first of its loads in a frame.
extern "C" void dino_interp_camera(uint8_t* rdram, recomp_context* ctx) {
    gpr address = ctx->r16;
    uint32_t index = std::min(((uint32_t)address - cameras_address) / camera_size, camera_count - 1);
    Camera& camera = cameras[index];
    gpr gfx = (gpr)(int32_t)MEM_W(0, ctx->r5);
    if (camera.frame != frame_number) {
        Position position = read_position(rdram, address + 0x44);
        int yaw = MEM_HU(0x50, address), pitch = MEM_HU(0x52, address), roll = MEM_HU(0x54, address);
        bool moved = camera.move.jumped(distance(position, camera.position), jump_distance);
        bool swung = camera.turn.jumped((float)std::max({ turned(camera.yaw, yaw), turned(camera.pitch, pitch), turned(camera.roll, roll) }), jump_turn);
        if (moved || swung) {
            camera.cuts++;
            if (index == 0) {
                world_cuts++;
                write_group(rdram, &gfx, 0x57000000 | (world_cuts & 0xFFFFFF), mode_decompose, false, false, component_auto, component_skip, order_auto);
            }

            if (logging) {
                fprintf(stderr, "cut: camera %u, frame %u, moved %.1f, turned %.0f\n", index, frame_number, camera.move.last, camera.turn.last);
            }
        }

        camera.position = position;
        camera.yaw = yaw;
        camera.pitch = pitch;
        camera.roll = roll;
        camera.frame = frame_number;
    }

    write_group(rdram, &gfx, 0x43000000 | (index << 16) | (camera.cuts & 0xFFFF), mode_simple, false, true, component_interpolate, component_skip, order_auto);
    MEM_W(0, ctx->r5) = (int32_t)gfx;
}

// In func_80035000, which draws an object, once it has a model: s0 is the object, s1 its model, and the function keeps
// the list's pointer at sp + 0x68. The object's position is at +0xC, as func_80004024 reads it.
extern "C" void dino_interp_object_begin(uint8_t* rdram, recomp_context* ctx) {
    gpr address = ctx->r16;
    Object& object = objects[(uint32_t)address];
    if (object.frame != frame_number) {
        Position position = read_position(rdram, address + 0xC);
        if (object.move.jumped(distance(position, object.position), jump_distance)) {
            object.jumps++;
            if (logging) {
                fprintf(stderr, "jump: object %08X, frame %u, moved %.1f\n", (uint32_t)address, frame_number, object.move.last);
            }
        }

        object.position = position;
        object.frame = frame_number;
    }

    gpr gfx = (gpr)(int32_t)MEM_W(0x68, ctx->r29);
    write_group(rdram, &gfx, object_id((uint32_t)address, (uint32_t)ctx->r17, object.jumps), mode_decompose, true, false,
        origin_stepped ? component_interpolate : component_auto, component_interpolate, order_linear);
    MEM_W(0x68, ctx->r29) = (int32_t)gfx;
}

// Where func_80035000 is done with the model: t2 is the list's pointer, on both ways there.
extern "C" void dino_interp_object_end(uint8_t* rdram, recomp_context* ctx) {
    gpr gfx = ctx->r10;
    write(rdram, &gfx, op_pop_group, 1);
    ctx->r10 = gfx;
}
