#include "controller.hpp"
#include "plugin_ids.hpp"
#include "processor.hpp"
#include "version.hpp"

#include "public.sdk/source/main/pluginfactory.h"

using namespace Steinberg;
using namespace Steinberg::Vst;

#define stringPluginName "PSX Reverb"

BEGIN_FACTORY_DEF(stringCompanyName, stringCompanyWeb, stringCompanyEmail)

DEF_CLASS2(
    INLINE_UID_FROM_FUID(psx_reverb::vst3::processorUid),
    PClassInfo::kManyInstances,
    kVstAudioEffectClass,
    stringPluginName,
    Vst::kDistributable,
    "Fx|Reverb",
    FULL_VERSION_STR,
    kVstVersionString,
    psx_reverb::vst3::Processor::create)

DEF_CLASS2(
    INLINE_UID_FROM_FUID(psx_reverb::vst3::controllerUid),
    PClassInfo::kManyInstances,
    kVstComponentControllerClass,
    stringPluginName " Controller",
    0,
    "",
    FULL_VERSION_STR,
    kVstVersionString,
    psx_reverb::vst3::Controller::create)

END_FACTORY
