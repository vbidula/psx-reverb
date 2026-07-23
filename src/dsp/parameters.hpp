#pragma once

#include <cstddef>
#include <cstdint>

namespace psx_reverb {

enum class Preset : std::uint8_t {
    room,
    studio_small,
    studio_medium,
    studio_large,
    hall,
    half_echo,
    space_echo,
    chaos_echo,
    delay,
    off,
    count,
};

inline constexpr std::size_t kPresetCount =
    static_cast<std::size_t>(Preset::count);
inline constexpr float kMinimumLevelDb = -30.0F;
inline constexpr float kMaximumLevelDb = 12.0F;

struct Parameters {
    float wet_db = 0.0F;
    float dry_db = 0.0F;
    float master_db = 0.0F;
    Preset preset = Preset::room;
};

} // namespace psx_reverb
