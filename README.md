# PSX Reverb DSP

PSX Reverb DSP is a host-independent C++17 implementation of the original
PlayStation SPU reverb algorithm. It is intended to be shared by plug-in and
game-engine adapters without coupling the audio processing code to a host API.

The implementation retains all 32 SPU reverb registers and the ten standard
presets found in many commercial PlayStation games:

1. Room
2. Studio Small
3. Studio Medium
4. Studio Large
5. Hall
6. Half Echo
7. Space Echo
8. Chaos Echo
9. Delay
10. Off

The algorithm uses floating-point delay memory and adapts the native 22.05 kHz
SPU delay addresses and IIR response to the configured host sample rate. It is
therefore an emulation of the SPU reverb topology rather than a bit-exact SPU
implementation.

## Build

Requirements:

- CMake 3.24 or newer
- A C++17 compiler

```sh
cmake --preset debug
cmake --build --preset debug
```

The build produces the position-independent static library
`psx_reverb_dsp`, also available to parent CMake projects as
`psx_reverb::dsp`.

## Basic use

```cpp
#include <dsp/psx_reverb.hpp>

psx_reverb::PsxReverb reverb;
reverb.prepare(48000.0F);

psx_reverb::Parameters parameters;
parameters.preset = psx_reverb::Preset::studio_large;
parameters.wet_db = -9.37F;
reverb.set_parameters(parameters);

reverb.process(
    left_input,
    right_input,
    left_output,
    right_output,
    frame_count);
```

`prepare()` allocates delay memory and must be called before processing and
away from the audio thread. `process()` is the planar block interface intended
for VST3. `process_sample()` is convenient for Godot's interleaved
`AudioFrame` data. Neither processing function allocates, blocks, or performs
I/O.

The host adapter owns and validates its parameter state, then supplies a
complete `Parameters` value through `set_parameters()`. Changing the preset
clears the current reverb tail.

## Source layout

```text
src/dsp/parameters.hpp            Public parameters and preset identifiers
src/dsp/psx_reverb.hpp            Host-independent DSP interface
src/dsp/psx_reverb.cpp            Reverb processing and register conversion
src/dsp/psx_reverb_presets.hpp    Complete standard SPU register tables
```

## License and provenance

The original plug-in implementation was written by Michael Panzlaff and
derived from example code by David Robillard and Steve Harris. The retained
DSP is distributed under the permissive license in [COPYING](COPYING).
