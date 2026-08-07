#pragma once

#include "dsp/psx_reverb.hpp"

#include "public.sdk/source/vst/vstaudioeffect.h"

namespace psx_reverb::vst3 {

class Processor final : public Steinberg::Vst::AudioEffect {
public:
    Processor();

    static Steinberg::FUnknown* create(void*);

    Steinberg::tresult PLUGIN_API initialize(
        Steinberg::FUnknown* context) override;
    Steinberg::tresult PLUGIN_API setBusArrangements(
        Steinberg::Vst::SpeakerArrangement* inputs,
        Steinberg::int32 input_count,
        Steinberg::Vst::SpeakerArrangement* outputs,
        Steinberg::int32 output_count) override;
    Steinberg::tresult PLUGIN_API setupProcessing(
        Steinberg::Vst::ProcessSetup& setup) override;
    Steinberg::tresult PLUGIN_API setActive(Steinberg::TBool active) override;
    Steinberg::tresult PLUGIN_API canProcessSampleSize(
        Steinberg::int32 symbolic_sample_size) override;
    Steinberg::uint32 PLUGIN_API getTailSamples() override;
    Steinberg::tresult PLUGIN_API process(
        Steinberg::Vst::ProcessData& data) override;
    Steinberg::tresult PLUGIN_API setState(Steinberg::IBStream* state) override;
    Steinberg::tresult PLUGIN_API getState(Steinberg::IBStream* state) override;

private:
    void readParameterChanges(
        Steinberg::Vst::IParameterChanges* changes) noexcept;

    PsxReverb reverb_;
    Parameters parameters_;
    bool bypass_ = false;
};

} // namespace psx_reverb::vst3
