#pragma once

#include "public.sdk/source/vst/vsteditcontroller.h"

namespace psx_reverb::vst3 {

class Controller final : public Steinberg::Vst::EditController {
public:
    static Steinberg::FUnknown* create(void*);

    Steinberg::tresult PLUGIN_API initialize(
        Steinberg::FUnknown* context) override;
    Steinberg::tresult PLUGIN_API setComponentState(
        Steinberg::IBStream* state) override;
    Steinberg::IPlugView* PLUGIN_API createView(
        Steinberg::FIDString name) override;
};

} // namespace psx_reverb::vst3
