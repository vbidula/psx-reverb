#include "controller.hpp"

#include "plugin_ids.hpp"

#include "base/source/fstreamer.h"
#include "pluginterfaces/base/ibstream.h"
#include "public.sdk/source/vst/vstparameters.h"
#include "vstgui/plugin-bindings/vst3editor.h"

#include <algorithm>
#include <string_view>

namespace psx_reverb::vst3 {

using namespace Steinberg;
using namespace Steinberg::Vst;

FUnknown* Controller::create(void*)
{
    return static_cast<IEditController*>(new Controller);
}

tresult PLUGIN_API Controller::initialize(FUnknown* const context)
{
    const auto result = EditController::initialize(context);
    if (result != kResultOk) {
        return result;
    }

    parameters.addParameter(new RangeParameter(
        STR16("Wet"),
        wetId,
        STR16("dB"),
        kMinimumLevelDb,
        kMaximumLevelDb,
        0.0));
    parameters.addParameter(new RangeParameter(
        STR16("Dry"),
        dryId,
        STR16("dB"),
        kMinimumLevelDb,
        kMaximumLevelDb,
        0.0));
    parameters.addParameter(new RangeParameter(
        STR16("Master"),
        masterId,
        STR16("dB"),
        kMinimumLevelDb,
        kMaximumLevelDb,
        0.0));

    auto* const preset = new StringListParameter(STR16("Preset"), presetId);
    preset->appendString(STR16("Room"));
    preset->appendString(STR16("Studio Small"));
    preset->appendString(STR16("Studio Medium"));
    preset->appendString(STR16("Studio Large"));
    preset->appendString(STR16("Hall"));
    preset->appendString(STR16("Half Echo"));
    preset->appendString(STR16("Space Echo"));
    preset->appendString(STR16("Chaos Echo"));
    preset->appendString(STR16("Delay"));
    preset->appendString(STR16("Off"));
    parameters.addParameter(preset);

    parameters.addParameter(
        STR16("Bypass"),
        nullptr,
        1,
        0.0,
        ParameterInfo::kCanAutomate | ParameterInfo::kIsBypass,
        bypassId);

    return kResultOk;
}

tresult PLUGIN_API Controller::setComponentState(IBStream* const state)
{
    if (!state) {
        return kResultFalse;
    }

    IBStreamer stream(state, kLittleEndian);
    float wet = 0.0F;
    float dry = 0.0F;
    float master = 0.0F;
    int32 preset = 0;
    int32 bypass = 0;

    if (!stream.readFloat(wet)
        || !stream.readFloat(dry)
        || !stream.readFloat(master)
        || !stream.readInt32(preset)
        || !stream.readInt32(bypass)) {
        return kResultFalse;
    }

    preset = std::clamp(preset, 0, static_cast<int32>(kPresetCount - 1));
    setParamNormalized(wetId, levelToNormalized(wet));
    setParamNormalized(dryId, levelToNormalized(dry));
    setParamNormalized(masterId, levelToNormalized(master));
    setParamNormalized(
        presetId, presetToNormalized(static_cast<Preset>(preset)));
    setParamNormalized(bypassId, bypass != 0 ? 1.0 : 0.0);
    return kResultOk;
}

IPlugView* PLUGIN_API Controller::createView(const FIDString name)
{
    if (name && std::string_view(name) == ViewType::kEditor) {
        return new VSTGUI::VST3Editor(this, "Editor", "editor.uidesc");
    }
    return nullptr;
}

} // namespace psx_reverb::vst3
