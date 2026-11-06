// The free camera and the mouse. The game has no camera turn of its own: dll2, its camera manager, ticks the camera
// mode's DLL, then dll2_1770 copies the mode's state (yaw +0, pitch +2, position +0xC) into the camera the world is
// drawn from. With Free Camera on, us.toml's hook at dll2_1770's start calls dino_camera_turn, which turns that state
// around its focus object (+0xA0): the mode goes on from the view the player chose, and the player's own code reads
// the camera for which way the stick walks. A camera mode other than the one that follows the player (a cutscene's, a
// first person view) is left alone, and a turned view keeps to the player's side of the walls (stop_at_walls).
//
// The turn comes from the right stick and the mouse. While Free Camera is on the right stick no longer plays the C
// buttons (recompinput::set_right_analog_suppressed); the D-pad plays them, as the arrows do on the keyboard.
// RecompFrontend locks the pointer while Mouse Sensitivity is above 0% and hands a game only the motion since its last
// poll, so the motion is added up here from every SDL event until the camera's tick takes it, and what wasn't taken for
// 100 ms (a cutscene, a pause) is dropped. The left button is B and the right Z on controller 1.

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstring>
#include <mutex>
#include <string>
#include <vector>

#include "SDL.h"

#include "recomp.h"
#include "recompinput/input_mapping.h"
#include "recompinput/input_state.h"
#include "recompinput/recompinput.h"
#include "recompui/config.h"
#include "dino.h"

extern "C" {
    void func_80053438(uint8_t* rdram, recomp_context* ctx);
    void func_80053750(uint8_t* rdram, recomp_context* ctx);
    void func_8005509C(uint8_t* rdram, recomp_context* ctx);
}

namespace {
    constexpr auto stale = std::chrono::milliseconds(100);
    constexpr float pi = 3.14159265f;
    constexpr float binary_angle = 32768.0f / pi; // The game's angles: 0x10000 a turn.
    constexpr uint16_t n64_button_b = 0x4000;
    constexpr uint16_t n64_button_z = 0x2000;

    // The follow camera's mode, dll2's bss + 0x178.
    constexpr int32_t follow_mode = 84;
    // Radians a second at a full stick, and a count at 50% Mouse Sensitivity.
    constexpr float stick_speed = 150.0f * pi / 180.0f;
    constexpr float mouse_speed = 0.1f * pi / 180.0f;
    // How far the view may look down and up, as the state's pitch.
    constexpr int32_t pitch_down = 0x2800;
    constexpr int32_t pitch_up = -0x1000;
    // How much of the way to a wall the camera comes each tick.
    constexpr float wall_ease = 0.25f;

    std::mutex mutex;
    float pending_dx = 0.0f;
    float pending_dy = 0.0f;
    std::chrono::steady_clock::time_point last_take{};
    std::atomic<float> scripted_x = 0.0f;
    std::atomic<float> scripted_y = 0.0f;
    bool turned = false;

    const std::string free_camera = "free_camera";
    const std::string invert_camera_y = "invert_camera_y";
    enum class FreeCamera {
        Original,
        On,
        OptionCount
    };
    const std::vector<recomp::config::ConfigOptionEnumOption> free_camera_options = {
        { FreeCamera::Original, "Original" },
        { FreeCamera::On, "On" },
    };
    enum class InvertY {
        Off,
        On,
        OptionCount
    };
    const std::vector<recomp::config::ConfigOptionEnumOption> invert_y_options = {
        { InvertY::Off, "Off" },
        { InvertY::On, "On" },
    };

    bool general_option_is(const std::string& id, uint32_t value) {
        return std::get<uint32_t>(recompui::config::get_general_config().get_option_value(id)) == value;
    }

    bool game_has_mouse() {
        return !recompinput::game_input_disabled() && SDL_GetRelativeMouseMode() == SDL_TRUE;
    }

    int watch_events(void*, SDL_Event* event) {
        if (event->type == SDL_MOUSEMOTION && game_has_mouse()) {
            std::lock_guard lock{ mutex };
            pending_dx += event->motion.xrel;
            pending_dy += event->motion.yrel;
        }
        return 0;
    }

    float read_float(uint8_t* rdram, gpr address) {
        int32_t word = MEM_W(0, address);
        float value;
        std::memcpy(&value, &word, sizeof(value));
        return value;
    }

    void write_float(uint8_t* rdram, gpr address, float value) {
        int32_t word;
        std::memcpy(&word, &value, sizeof(word));
        MEM_W(0, address) = word;
    }

    // The turn since the last tick in radians, right and down positive (up with Invert Camera Y).
    void take_turn(float* yaw, float* pitch) {
        static auto last_tick = std::chrono::steady_clock::now();
        auto now = std::chrono::steady_clock::now();
        float seconds = std::min(std::chrono::duration<float>(now - last_tick).count(), 0.1f);
        last_tick = now;

        float stick_x = 0.0f;
        float stick_y = 0.0f;
        if (!recompinput::game_input_disabled()) {
            recompinput::get_right_analog(0, &stick_x, &stick_y);
        }
        stick_x += scripted_x.load();
        stick_y += scripted_y.load();

        float mouse_x = 0.0f;
        float mouse_y = 0.0f;
        {
            std::lock_guard lock{ mutex };
            if (now - last_take < stale) {
                mouse_x = pending_dx;
                mouse_y = pending_dy;
            }
            pending_dx = 0.0f;
            pending_dy = 0.0f;
            last_take = now;
        }
        float mouse_scale = mouse_speed * (float)(recompui::config::general::get_mouse_sensitivity() / 50.0);

        *yaw = stick_x * stick_speed * seconds + mouse_x * mouse_scale;
        *pitch = stick_y * stick_speed * seconds + mouse_y * mouse_scale;
        if (general_option_is(invert_camera_y, (uint32_t)InvertY::On)) {
            *pitch = -*pitch;
        }
    }
}

void dino::init_camera() {
    using recompinput::GameInput;
    using recompinput::InputField;
    // The C buttons' defaults on a pad, the D-pad beside the right stick: the free camera takes the stick.
    recompinput::set_default_mapping_for_controller(GameInput::C_LEFT, {
        InputField::controller_analog(SDL_CONTROLLER_AXIS_RIGHTX, false),
        InputField::controller_digital(SDL_CONTROLLER_BUTTON_DPAD_LEFT) });
    recompinput::set_default_mapping_for_controller(GameInput::C_RIGHT, {
        InputField::controller_analog(SDL_CONTROLLER_AXIS_RIGHTX, true),
        InputField::controller_digital(SDL_CONTROLLER_BUTTON_DPAD_RIGHT) });
    recompinput::set_default_mapping_for_controller(GameInput::C_UP, {
        InputField::controller_analog(SDL_CONTROLLER_AXIS_RIGHTY, false),
        InputField::controller_digital(SDL_CONTROLLER_BUTTON_DPAD_UP) });
    recompinput::set_default_mapping_for_controller(GameInput::C_DOWN, {
        InputField::controller_analog(SDL_CONTROLLER_AXIS_RIGHTY, true),
        InputField::controller_digital(SDL_CONTROLLER_BUTTON_DPAD_DOWN) });
    for (GameInput input : { GameInput::DPAD_LEFT, GameInput::DPAD_RIGHT, GameInput::DPAD_UP, GameInput::DPAD_DOWN }) {
        recompinput::set_default_mapping_for_controller(input, {});
    }
    SDL_AddEventWatch(watch_events, nullptr);
}

void dino::add_camera_options(recomp::config::Config& general) {
    general.add_enum_option(
        free_camera,
        "Free Camera",
        "The right stick and the mouse turn the camera around the player, and the D-pad plays the C buttons in the "
        "stick's place. <b>Original is the console's camera</b>: it follows by itself, and the right stick plays the "
        "C buttons.",
        free_camera_options,
        FreeCamera::On
    );
    // The frontend's own option, by its id: the frontend locks the pointer while it is above zero.
    general.add_percent_number_option(
        recompui::config::general::options::mouse_sensitivity,
        "Mouse Sensitivity",
        "How far the mouse turns the camera: at 50% a count turns it 0.1 degrees. Left click is B and right click Z. "
        "<b>0% is the original game</b>: no mouse, and the pointer stays free.",
        50.0
    );
    general.add_enum_option(
        invert_camera_y,
        "Invert Camera Y",
        "Moving the mouse or the right stick up looks down.",
        invert_y_options,
        InvertY::Off
    );
}

void dino::add_mouse_buttons(uint16_t* buttons) {
    if (game_has_mouse()) {
        uint32_t mouse = SDL_GetMouseState(nullptr, nullptr);
        if (mouse & SDL_BUTTON(SDL_BUTTON_LEFT)) {
            *buttons |= n64_button_b;
        }
        if (mouse & SDL_BUTTON(SDL_BUTTON_RIGHT)) {
            *buttons |= n64_button_z;
        }
    }
}

void dino::set_scripted_camera(float x, float y) {
    scripted_x = x;
    scripted_y = y;
}

// Turns the mode's state around its focus.
static void turn_around_focus(uint8_t* rdram, gpr state, gpr focus, float yaw_turn, float pitch_turn) {
    float focus_x = read_float(rdram, focus + 0x18);
    float focus_y = read_float(rdram, focus + 0x1C);
    float focus_z = read_float(rdram, focus + 0x20);
    float x = read_float(rdram, state + 0xC) - focus_x;
    float y = read_float(rdram, state + 0x10) - focus_y;
    float z = read_float(rdram, state + 0x14) - focus_z;

    // Around the focus: the state's yaw is the camera's bearing from it, turned the same way.
    float cosine = std::cos(yaw_turn);
    float sine = std::sin(yaw_turn);
    float turned_x = x * cosine + z * sine;
    float turned_z = z * cosine - x * sine;
    int32_t yaw = MEM_H(0, state);
    MEM_H(0, state) = (int16_t)(yaw + (int32_t)(yaw_turn * binary_angle));

    // Up and down: the camera rises or falls on its circle around the focus, as far as the pitch allows.
    int32_t pitch = MEM_H(2, state);
    int32_t new_pitch = std::clamp(pitch + (int32_t)(pitch_turn * binary_angle), pitch_up, pitch_down);
    float pitched = (float)(new_pitch - pitch) / binary_angle;
    float flat = std::sqrt(turned_x * turned_x + turned_z * turned_z);
    if (flat > 1.0f && new_pitch != pitch) {
        float new_flat = flat * std::cos(pitched) - y * std::sin(pitched);
        y = y * std::cos(pitched) + flat * std::sin(pitched);
        turned_x *= new_flat / flat;
        turned_z *= new_flat / flat;
        MEM_H(2, state) = (int16_t)new_pitch;
    }

    write_float(rdram, state + 0xC, focus_x + turned_x);
    write_float(rdram, state + 0x10, focus_y + y);
    write_float(rdram, state + 0x14, focus_z + turned_z);
}

// The game's own collision, as the mode's dll84_1A58 calls it, on the line from the focus out to the camera at the
// camera's height: the mode only sweeps each tick's step, which a turn would carry through a wall, so the camera comes
// in to the first wall between it and the player, a part of the way each tick so a turn doesn't jump. The mode's own
// collision record (state + 0x34) is set up by its tick.
static void stop_at_walls(uint8_t* rdram, recomp_context* ctx, gpr state, gpr focus) {
    recomp_context saved = *ctx;
    gpr sp = ctx->r29 - 0x80;
    gpr box = sp + 0x20;
    gpr from = sp + 0x40;
    gpr to = sp + 0x50;
    float x = read_float(rdram, state + 0xC);
    float y = read_float(rdram, state + 0x10);
    float z = read_float(rdram, state + 0x14);
    float focus_x = read_float(rdram, focus + 0x18);
    float focus_z = read_float(rdram, focus + 0x20);
    write_float(rdram, from, focus_x);
    write_float(rdram, from + 4, y);
    write_float(rdram, from + 8, focus_z);
    write_float(rdram, to, x);
    write_float(rdram, to + 4, y);
    write_float(rdram, to + 8, z);

    ctx->r29 = sp;
    ctx->r4 = box;
    ctx->r5 = from;
    ctx->r6 = to;
    ctx->r7 = state + 0x74;
    MEM_W(0x10, sp) = 1;
    func_80053438(rdram, ctx);
    ctx->r4 = focus;
    ctx->r5 = box;
    ctx->r6 = 1;
    func_80053750(rdram, ctx);
    ctx->r4 = focus;
    ctx->r5 = from;
    ctx->r6 = to;
    ctx->r7 = 1;
    MEM_W(0x10, sp) = (int32_t)(state + 0x34);
    MEM_W(0x14, sp) = 0;
    func_8005509C(rdram, ctx);
    *ctx = saved;

    float stopped_x = read_float(rdram, to);
    float stopped_z = read_float(rdram, to + 8);
    if (stopped_x != x || stopped_z != z) {
        float eased_x = x + (stopped_x - x) * wall_ease;
        float eased_z = z + (stopped_z - z) * wall_ease;
        write_float(rdram, state + 0xC, eased_x);
        write_float(rdram, state + 0x14, eased_z);
        // The view looks at the focus from where the camera is now.
        MEM_H(0, state) = (int16_t)(0x8000 - (int32_t)(std::atan2(eased_x - focus_x, eased_z - focus_z) * binary_angle));
    }
}

// $a0 is the camera mode's state; dll2's $gp is set, its GOT's 0xC entry being the address of its bss.
extern "C" void dino_camera_turn(uint8_t* rdram, recomp_context* ctx) {
    bool on = general_option_is(free_camera, (uint32_t)FreeCamera::On);
    recompinput::set_right_analog_suppressed(on);
    gpr state = ctx->r4;
    gpr focus = (gpr)MEM_W(0xA0, state);
    gpr bss = (gpr)MEM_W(0xC, ctx->r28);
    float yaw_turn;
    float pitch_turn;
    take_turn(&yaw_turn, &pitch_turn);
    if (!on || focus == 0 || MEM_W(0x178, bss) != follow_mode) {
        turned = false;
        return;
    }
    if (yaw_turn != 0.0f || pitch_turn != 0.0f) {
        turn_around_focus(rdram, state, focus, yaw_turn, pitch_turn);
        turned = true;
    }
    // Until the view is turned it is the console's, which stands behind walls that are drawn from one side only.
    if (turned) {
        stop_at_walls(rdram, ctx, state, focus);
    }
}
