#include "processor.hpp"

#include "plugin_ids.hpp"

#include "base/source/fstreamer.h"
#include "pluginterfaces/base/ibstream.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"

#include <algorithm>

namespace psx_reverb::vst3 {

using namespace Steinberg;
using namespace Steinberg::Vst;

Processor::Processor()
{
    setControllerClass(controllerUid);
}

FUnknown* Processor::create(void*)
{
    return static_cast<IAudioProcessor*>(new Processor);
}

tresult PLUGIN_API Processor::initialize(FUnknown* const context)
{
    const auto result = AudioEffect::initialize(context);
    if (result != kResultOk) {
        return result;
    }

    addAudioInput(STR16("Stereo In"), SpeakerArr::kStereo);
    addAudioOutput(STR16("Stereo Out"), SpeakerArr::kStereo);
    return kResultOk;
}

tresult PLUGIN_API Processor::setBusArrangements(
    SpeakerArrangement* const inputs,
    const int32 input_count,
    SpeakerArrangement* const outputs,
    const int32 output_count)
{
    if (input_count != 1 || output_count != 1
        || inputs[0] != SpeakerArr::kStereo
        || outputs[0] != SpeakerArr::kStereo) {
        return kResultFalse;
    }

    return AudioEffect::setBusArrangements(
        inputs, input_count, outputs, output_count);
}

tresult PLUGIN_API Processor::setupProcessing(ProcessSetup& setup)
{
    const auto result = AudioEffect::setupProcessing(setup);
    if (result == kResultOk) {
        reverb_.prepare(static_cast<float>(setup.sampleRate));
        reverb_.set_parameters(parameters_);
    }
    return result;
}

tresult PLUGIN_API Processor::setActive(const TBool active)
{
    if (active) {
        reverb_.reset();
    }
    return AudioEffect::setActive(active);
}

tresult PLUGIN_API Processor::canProcessSampleSize(
    const int32 symbolic_sample_size)
{
    return symbolic_sample_size == kSample32 ? kResultTrue : kResultFalse;
}

uint32 PLUGIN_API Processor::getTailSamples()
{
    return kInfiniteTail;
}

void Processor::readParameterChanges(IParameterChanges* const changes) noexcept
{
    if (!changes) {
        return;
    }

    for (int32 index = 0; index < changes->getParameterCount(); ++index) {
        auto* const queue = changes->getParameterData(index);
        if (!queue || queue->getPointCount() == 0) {
            continue;
        }

        int32 sample_offset = 0;
        ParamValue value = 0.0;
        for (int32 point = 0; point < queue->getPointCount(); ++point) {
            queue->getPoint(point, sample_offset, value);
        }

        switch (queue->getParameterId()) {
        case wetId:
            parameters_.wet_db = levelFromNormalized(value);
            break;
        case dryId:
            parameters_.dry_db = levelFromNormalized(value);
            break;
        case masterId:
            parameters_.master_db = levelFromNormalized(value);
            break;
        case presetId:
            parameters_.preset = presetFromNormalized(value);
            break;
        case bypassId:
            bypass_ = value > 0.5;
            break;
        default:
            break;
        }
    }

    reverb_.set_parameters(parameters_);
}

tresult PLUGIN_API Processor::process(ProcessData& data)
{
    readParameterChanges(data.inputParameterChanges);

    if (data.numSamples == 0) {
        return kResultOk;
    }
    if (data.numInputs != 1 || data.numOutputs != 1) {
        return kResultFalse;
    }

    auto** const input = data.inputs[0].channelBuffers32;
    auto** const output = data.outputs[0].channelBuffers32;

    if (bypass_) {
        std::copy_n(input[0], data.numSamples, output[0]);
        std::copy_n(input[1], data.numSamples, output[1]);
        data.outputs[0].silenceFlags = data.inputs[0].silenceFlags;
        return kResultOk;
    }

    reverb_.process(
        input[0],
        input[1],
        output[0],
        output[1],
        static_cast<std::size_t>(data.numSamples));
    data.outputs[0].silenceFlags = 0;
    return kResultOk;
}

tresult PLUGIN_API Processor::setState(IBStream* const state)
{
    if (!state) {
        return kResultFalse;
    }

    IBStreamer stream(state, kLittleEndian);
    int32 preset = 0;
    int32 bypass = 0;

    if (!stream.readFloat(parameters_.wet_db)
        || !stream.readFloat(parameters_.dry_db)
        || !stream.readFloat(parameters_.master_db)
        || !stream.readInt32(preset)
        || !stream.readInt32(bypass)) {
        return kResultFalse;
    }

    preset = std::clamp(preset, 0, static_cast<int32>(kPresetCount - 1));
    parameters_.preset = static_cast<Preset>(preset);
    bypass_ = bypass != 0;
    reverb_.set_parameters(parameters_);
    return kResultOk;
}

tresult PLUGIN_API Processor::getState(IBStream* const state)
{
    if (!state) {
        return kResultFalse;
    }

    IBStreamer stream(state, kLittleEndian);
    const bool saved =
        stream.writeFloat(parameters_.wet_db)
        && stream.writeFloat(parameters_.dry_db)
        && stream.writeFloat(parameters_.master_db)
        && stream.writeInt32(static_cast<int32>(parameters_.preset))
        && stream.writeInt32(bypass_ ? 1 : 0);
    return saved ? kResultOk : kResultFalse;
}

} // namespace psx_reverb::vst3
