// CloudRunner Fortress (map 12): the barrels' way to the courtyard roof. The roof, CFBlastedRoof, blows at the third
// CFbarrel blown under it, and the barrels stand in the room behind the explosive key's door, west of the blasted wall.
// Their only way to the courtyard is over the treasure shaft, on the wind of CFTreasWindLift, which holds an
// empty-handed player at the corridor's height but drops one carrying a barrel to the shaft's bottom, so the roof
// can't be blown (docs/story-points.md, CloudRunner Fortress). With Wind Bridge on Restored (the default) the wind
// holds a player carrying a barrel as it holds one empty-handed; on Original she falls, as on the console.
//
// Read from the ROM (offsets are a DLL's text):
// - CFBlastedRoof (dll389, placement 2917 at (-12282, 2189, 18881), group 8 of 0x458; setup +18: 0000 0002 0001 02e1
//   0900) counts hits of type 5, a CFbarrel's explosion (dll388 0x12A4-0x1314: obj+0xA8 100, 10 up), each from a new
//   source (its list at D_0638). Each moves the bit 0x2DE+n to 0x2DF+n and the count 0x900 to n+1; the hit that finds
//   n at its +0x1A (2) sets 0x2E1, its +0x1E, and blows (dll389 0x01CC-0x0398). So three barrels, and a barrel blown on
//   the courtyard's floor under the roof's middle, about 100 below it, reaches it. The room behind the key's door has
//   four barrels: one for the cracked wall, three for the roof.
// - CFTreasWindLift (dll370, in group 13 of 0x458) pushes the player each frame through the player DLL's dll210.58
//   (dll370 0x0E00-0x0E10), which only stores the speed at her state's +0x808. Only dll210_BA38, the walking states'
//   ground checks, reads it, and sends her into the wind's ride, state 0x49 (BA38 0x0C134-0x0C14C returns 0x4A).
// - The walking states, 1 (dll210_A3FC), 3 (AA80) and 4 (AE34), skip BA38 while she carries something (state+0x868),
//   going straight to whether the object still holds on (+0x870, which it confirms each frame) and else letting go
//   (A3FC 0xA470, AA80 0xAAB4, AE34 0xAF28). So a carrying player is never put on the wind, and walks off the lip.
//
// What Restored changes: us.toml's hooks on those three carry paths send a player who carries something still held
// into state 0x49 while the wind pushes her, the way BA38 sends one empty-handed. The ride and the fall after it don't
// touch what she carries, so she lands still holding the barrel. Treasure Lift (cloudrunner.cpp) leaves a player who
// carries something to the wind, so with both on Restored a barrel crosses the shaft and an empty-handed player is
// lowered to the treasure caves.

#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

#include "recomp.h"
#include "recompui/config.h"
#include "dino.h"

namespace {
    const std::string wind_bridge = "wind_bridge";

    enum class WindBridge {
        Original,
        Restored,
        OptionCount
    };
    const std::vector<recomp::config::ConfigOptionEnumOption> wind_bridge_options = {
        { WindBridge::Original, "Original" },
        { WindBridge::Restored, "Restored" },
    };

    bool restored() {
        return std::get<uint32_t>(recompui::config::get_general_config().get_option_value(wind_bridge)) ==
            (uint32_t)WindBridge::Restored;
    }
}

void dino::add_cloudrunner_barrel_options(recomp::config::Config& general) {
    general.add_enum_option(
        wind_bridge,
        "Wind Bridge",
        "In CloudRunner Fortress the wind over the shaft past the blasted wall holds a player carrying a barrel, so the "
        "barrels reach the courtyard and blow its roof. <b>Original is the prototype as it was left</b>: carrying one, "
        "she falls to the shaft's bottom.",
        wind_bridge_options,
        WindBridge::Restored
    );
}

// us.toml's hooks on the walking states' carry paths: whether the player, her state at `state`, carries something
// still held while the wind pushes her (+0x808 not zero, as BA38 compares it).
extern "C" int dino_carried_in_wind(uint8_t* rdram, uint32_t state) {
    gpr s = (int32_t)state;
    if (!restored() || MEM_W(0x868, s) == 0 || MEM_BU(0x870, s) == 0) {
        return 0;
    }
    uint32_t bits = MEM_W(0x808, s);
    float push;
    memcpy(&push, &bits, sizeof(push));
    return push != 0.0f;
}

// Whether the player object carries something, for Treasure Lift (cloudrunner.cpp).
bool dino::player_carries(uint8_t* rdram, int32_t player) {
    gpr state = (int32_t)MEM_W(0xB8, (gpr)player);
    return state != 0 && MEM_W(0x868, state) != 0;
}
