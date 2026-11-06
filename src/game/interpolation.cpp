// What RT64 needs to interpolate the game to the display's rate. us.toml's hooks call these with the game's display
// list pointer and each writes RT64's extended commands into the list (rt64_extended_gbi.h).

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <chrono>

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

    void write(uint8_t* rdram, gpr* gfx, uint32_t w0, uint32_t w1) {
        MEM_W(0, *gfx) = (int32_t)w0;
        MEM_W(4, *gfx) = (int32_t)w1;
        *gfx += 8;
    }

    void write_group(uint8_t* rdram, gpr* gfx, uint32_t id, uint32_t mode, bool push, bool proj, uint32_t pos, uint32_t rot, uint32_t scale,
        uint32_t skew, uint32_t persp, uint32_t vert, uint32_t tile, uint32_t order, uint32_t tc, uint32_t lookat) {
        write(rdram, gfx, op_matrix_group, id);
        write(rdram, gfx, (push ? 1 : 0) | ((proj ? 1 : 0) << 1) | (mode << 2) | (pos << 3) | (rot << 5) | (scale << 7) | (skew << 9) | (persp << 11) |
            (vert << 13) | (tile << 15) | (order << 17) | (aspect_auto << 20) | (tc << 22) | (lookat << 24), 0);
    }

    float read_float(uint8_t* rdram, gpr address) {
        int32_t bits = MEM_W(0, address);
        float value;
        memcpy(&value, &bits, sizeof(value));
        return value;
    }

    bool logging = getenv("DINO_INTERP_LOG") != nullptr;
    uint32_t camera_cut = 0;
    struct CameraState { float x, y, z; int yaw, pitch, roll; };
    CameraState cameras[4] = {};
    int objects = 0;
}

extern "C" void dino_interp_frame(uint8_t* rdram, recomp_context* ctx) {
    gpr gfx = (gpr)(int32_t)MEM_W(0, ctx->r4);
    write(rdram, &gfx, (hook_opcode << 24) | hook_magic, (hook_enable << 28) | extended_opcode);
    MEM_W(0, ctx->r4) = (int32_t)gfx;
    if (logging) {
        static auto last = std::chrono::steady_clock::now();
        auto now = std::chrono::steady_clock::now();
        double ms = std::chrono::duration<double, std::milli>(now - last).count();
        last = now;
        gpr index = (uint8_t)MEM_BU(0, (gpr)(int32_t)0x800B09C1);
        uint32_t start = (uint32_t)MEM_W(0, (gpr)(int32_t)(0x800AE678 + index * 4));
        fprintf(stderr, "frame %.1f ms objects %d list at %08X\n", ms, objects, start);
        objects = 0;
    }
}

extern "C" void dino_interp_list_end(uint8_t* rdram, recomp_context* ctx) {
    if (logging) {
        gpr index = (uint8_t)MEM_BU(0, (gpr)(int32_t)0x800B09C1);
        uint32_t mstart = (uint32_t)MEM_W(0, (gpr)(int32_t)(0x800AE688 + index * 4));
        uint32_t mend = (uint32_t)MEM_W(0, (gpr)(int32_t)0x800AE690);
        fprintf(stderr, "list %u of 36000 mtx %u of 70400\n", (uint32_t)(ctx->r5 - ctx->r4), mend - mstart);
    }
}

extern "C" void dino_interp_camera(uint8_t* rdram, recomp_context* ctx) {
    gpr camera = ctx->r16;
    float x = read_float(rdram, camera + 0x44), y = read_float(rdram, camera + 0x48), z = read_float(rdram, camera + 0x4C);
    int yaw = MEM_HU(0x50, camera), pitch = MEM_HU(0x52, camera), roll = MEM_HU(0x54, camera);
    uint32_t index = (uint32_t)(((uint32_t)camera - 0x800A6278) / 0x60) & 3;
    CameraState& last = cameras[index];
    if (logging) {
        float dx = x - last.x, dy = y - last.y, dz = z - last.z;
        fprintf(stderr, "camera %u pos %.1f %.1f %.1f moved %.1f yaw %d (%d) pitch %d (%d) roll %d (%d) proj00 %.3f origin %d %d\n", index, x, y, z,
            sqrtf(dx * dx + dy * dy + dz * dz), yaw, (int16_t)(yaw - last.yaw), pitch, (int16_t)(pitch - last.pitch), roll, (int16_t)(roll - last.roll),
            read_float(rdram, (gpr)(int32_t)0x800A6AE8), (int)read_float(rdram, (gpr)(int32_t)0x80092A70), (int)read_float(rdram, (gpr)(int32_t)0x80092A74));
    }
    last = { x, y, z, yaw, pitch, roll };
    gpr gfx = (gpr)(int32_t)MEM_W(0, ctx->r5);
    write_group(rdram, &gfx, 0x44500000 | (index << 16) | camera_cut, mode_simple, false, true, component_interpolate, component_interpolate, component_interpolate,
        component_interpolate, component_interpolate, component_skip, component_auto, order_auto, component_skip, component_auto);
    MEM_W(0, ctx->r5) = (int32_t)gfx;
}

extern "C" void dino_interp_object_begin(uint8_t* rdram, recomp_context* ctx) {
    gpr gfx = (gpr)(int32_t)MEM_W(0x68, ctx->r29);
    uint32_t id = 0x4F000000 | ((uint32_t)ctx->r16 & 0xFFFFFF);
    write_group(rdram, &gfx, id, mode_decompose, true, false, component_auto, component_auto, component_auto, component_auto, component_auto,
        component_interpolate, component_auto, order_linear, component_skip, component_auto);
    MEM_W(0x68, ctx->r29) = (int32_t)gfx;
    objects++;
}

extern "C" void dino_interp_object_end(uint8_t* rdram, recomp_context* ctx) {
    gpr gfx = ctx->r10;
    write(rdram, &gfx, op_pop_group, 1);
    ctx->r10 = gfx;
}
