#pragma once

#include "dsp/parameters.hpp"

#include "pluginterfaces/base/funknown.h"
#include "pluginterfaces/vst/vsttypes.h"

#include <algorithm>
#include <cstddef>

namespace psx_reverb::vst3 {

enum ParameterId : Steinberg::Vst::ParamID {
    wetId,
    dryId,
    masterId,
    presetId,
    bypassId,
};

static const Steinberg::FUID processorUid(
    0x9D343D65, 0xAF534CFD, 0x9A7377AD, 0x81AE378D);
static const Steinberg::FUID controllerUid(
    0xD32236BD, 0xE3E04920, 0xA3E1207F, 0x1D02F93C);

inline float levelFromNormalized(const Steinberg::Vst::ParamValue value) noexcept
{
    const auto normalized = std::clamp(value, 0.0, 1.0);
    return kMinimumLevelDb
        + static_cast<float>(normalized)
            * (kMaximumLevelDb - kMinimumLevelDb);
}

inline Steinberg::Vst::ParamValue levelToNormalized(const float value) noexcept
{
    return std::clamp(
        static_cast<double>(
            (value - kMinimumLevelDb)
            / (kMaximumLevelDb - kMinimumLevelDb)),
        0.0,
        1.0);
}

inline Preset presetFromNormalized(const Steinberg::Vst::ParamValue value) noexcept
{
    const auto index = std::min(
        kPresetCount - 1,
        static_cast<std::size_t>(
            std::clamp(value, 0.0, 1.0)
            * static_cast<double>(kPresetCount)));
    return static_cast<Preset>(index);
}

inline Steinberg::Vst::ParamValue presetToNormalized(const Preset preset) noexcept
{
    return static_cast<double>(preset) / static_cast<double>(kPresetCount - 1);
}

} // namespace psx_reverb::vst3
