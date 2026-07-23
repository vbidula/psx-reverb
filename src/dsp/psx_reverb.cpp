#include "dsp/psx_reverb.hpp"

#include "dsp/psx_reverb_presets.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>

namespace psx_reverb {
namespace {

constexpr float kSpuRate = 22050.0F;
constexpr std::size_t kLongestPreset = 0x18040U / 2U;
constexpr float kSmoothing = 0.001F;
constexpr double kPi = 3.14159265358979323846;

float q15(const std::uint16_t word) noexcept
{
    const auto value = word < 0x8000U
        ? static_cast<std::int32_t>(word)
        : static_cast<std::int32_t>(word) - 0x10000;
    return static_cast<float>(value) / 32768.0F;
}

float rescale_iir(const std::uint16_t word, const float sample_rate) noexcept
{
    const float alpha = q15(word);
    if (alpha == 0.0F) {
        return 0.0F;
    }

    const double spu_dt = 1.0 / kSpuRate;
    const float cutoff = static_cast<float>(
        1.0 / (2.0 * kPi * (spu_dt / alpha - spu_dt)));
    const double dt = 1.0 / sample_rate;
    const double time_constant = 1.0 / (2.0 * kPi * cutoff);
    return static_cast<float>(dt / (time_constant + dt));
}

float db_to_gain(const float db) noexcept
{
    return db > -90.0F ? std::pow(10.0F, db * 0.05F) : 0.0F;
}

std::size_t next_power_of_two(std::size_t value) noexcept
{
    --value;
    for (std::size_t shift = 1; shift < std::numeric_limits<std::size_t>::digits;
         shift *= 2) {
        value |= value >> shift;
    }
    return value + 1;
}

std::size_t address(const std::uint16_t word, const float stretch) noexcept
{
    return static_cast<std::size_t>(
        static_cast<float>(static_cast<std::uint32_t>(word) << 2U) * stretch);
}

} // namespace

void PsxReverb::prepare(const float sample_rate)
{
    sample_rate_ = sample_rate;
    const auto samples = static_cast<std::size_t>(std::ceil(
        static_cast<double>(kLongestPreset)
        * (static_cast<double>(sample_rate) / kSpuRate)));

    delay_memory_.assign(next_power_of_two(samples), 0.0F);
    delay_memory_mask_ = delay_memory_.size() - 1;
    buffer_address_ = 0;

    wet_target_ = db_to_gain(parameters_.wet_db);
    dry_target_ = db_to_gain(parameters_.dry_db);
    master_target_ = db_to_gain(parameters_.master_db);
    load_preset(parameters_.preset);

    wet_ = 1.0F;
    dry_ = 1.0F;
    master_ = 1.0F;
}

void PsxReverb::reset() noexcept
{
    std::fill(delay_memory_.begin(), delay_memory_.end(), 0.0F);
    buffer_address_ = 0;
    wet_ = 1.0F;
    dry_ = 1.0F;
    master_ = 1.0F;
}

void PsxReverb::set_parameters(const Parameters& parameters) noexcept
{
    const bool preset_changed = parameters.preset != parameters_.preset;
    parameters_ = parameters;

    wet_target_ = db_to_gain(parameters.wet_db);
    dry_target_ = db_to_gain(parameters.dry_db);
    master_target_ = db_to_gain(parameters.master_db);

    if (preset_changed && !delay_memory_.empty()) {
        load_preset(parameters.preset);
    }
}

StereoFrame PsxReverb::process_sample(
    const float left,
    const float right) noexcept
{
    wet_ += kSmoothing * (wet_target_ - wet_);
    dry_ += kSmoothing * (dry_target_ - dry_);
    master_ += kSmoothing * (master_target_ - master_);

    const float left_in = v_l_in_ * left;
    const float right_in = v_r_in_ * right;

    memory(m_l_same_) =
        (left_in + memory(d_l_same_) * v_wall_ - memory(m_l_same_ - 1))
            * v_iir_
        + memory(m_l_same_ - 1);
    memory(m_r_same_) =
        (right_in + memory(d_r_same_) * v_wall_ - memory(m_r_same_ - 1))
            * v_iir_
        + memory(m_r_same_ - 1);

    memory(m_l_diff_) =
        (left_in + memory(d_r_diff_) * v_wall_ - memory(m_l_diff_ - 1))
            * v_iir_
        + memory(m_l_diff_ - 1);
    memory(m_r_diff_) =
        (right_in + memory(d_l_diff_) * v_wall_ - memory(m_r_diff_ - 1))
            * v_iir_
        + memory(m_r_diff_ - 1);

    float left_out =
        v_comb1_ * memory(m_l_comb1_)
        + v_comb2_ * memory(m_l_comb2_)
        + v_comb3_ * memory(m_l_comb3_)
        + v_comb4_ * memory(m_l_comb4_);
    float right_out =
        v_comb1_ * memory(m_r_comb1_)
        + v_comb2_ * memory(m_r_comb2_)
        + v_comb3_ * memory(m_r_comb3_)
        + v_comb4_ * memory(m_r_comb4_);

    left_out -= v_apf1_ * memory(m_l_apf1_ - d_apf1_);
    memory(m_l_apf1_) = left_out;
    left_out = left_out * v_apf1_ + memory(m_l_apf1_ - d_apf1_);

    right_out -= v_apf1_ * memory(m_r_apf1_ - d_apf1_);
    memory(m_r_apf1_) = right_out;
    right_out = right_out * v_apf1_ + memory(m_r_apf1_ - d_apf1_);

    left_out -= v_apf2_ * memory(m_l_apf2_ - d_apf2_);
    memory(m_l_apf2_) = left_out;
    left_out = left_out * v_apf2_ + memory(m_l_apf2_ - d_apf2_);

    right_out -= v_apf2_ * memory(m_r_apf2_ - d_apf2_);
    memory(m_r_apf2_) = right_out;
    right_out = right_out * v_apf2_ + memory(m_r_apf2_ - d_apf2_);

    buffer_address_ = (buffer_address_ + 1) & delay_memory_mask_;

    return {
        (left_out * wet_ + left_in * dry_) * master_,
        (right_out * wet_ + right_in * dry_) * master_,
    };
}

void PsxReverb::process(
    const float* const left_in,
    const float* const right_in,
    float* const left_out,
    float* const right_out,
    const std::size_t frames) noexcept
{
    for (std::size_t frame = 0; frame < frames; ++frame) {
        const auto output = process_sample(left_in[frame], right_in[frame]);
        left_out[frame] = output.left;
        right_out[frame] = output.right;
    }
}

void PsxReverb::load_preset(const Preset preset) noexcept
{
    const auto& p = kPresets[static_cast<std::size_t>(preset)];
    const float stretch = sample_rate_ / kSpuRate;

    d_apf1_ = address(p[dAPF1], stretch);
    d_apf2_ = address(p[dAPF2], stretch);
    v_iir_ = rescale_iir(p[vIIR], sample_rate_);
    v_comb1_ = q15(p[vCOMB1]);
    v_comb2_ = q15(p[vCOMB2]);
    v_comb3_ = q15(p[vCOMB3]);
    v_comb4_ = q15(p[vCOMB4]);
    v_wall_ = q15(p[vWALL]);
    v_apf1_ = q15(p[vAPF1]);
    v_apf2_ = q15(p[vAPF2]);
    m_l_same_ = address(p[mLSAME], stretch);
    m_r_same_ = address(p[mRSAME], stretch);
    m_l_comb1_ = address(p[mLCOMB1], stretch);
    m_r_comb1_ = address(p[mRCOMB1], stretch);
    m_l_comb2_ = address(p[mLCOMB2], stretch);
    m_r_comb2_ = address(p[mRCOMB2], stretch);
    d_l_same_ = address(p[dLSAME], stretch);
    d_r_same_ = address(p[dRSAME], stretch);
    m_l_diff_ = address(p[mLDIFF], stretch);
    m_r_diff_ = address(p[mRDIFF], stretch);
    m_l_comb3_ = address(p[mLCOMB3], stretch);
    m_r_comb3_ = address(p[mRCOMB3], stretch);
    m_l_comb4_ = address(p[mLCOMB4], stretch);
    m_r_comb4_ = address(p[mRCOMB4], stretch);
    d_l_diff_ = address(p[dLDIFF], stretch);
    d_r_diff_ = address(p[dRDIFF], stretch);
    m_l_apf1_ = address(p[mLAPF1], stretch);
    m_r_apf1_ = address(p[mRAPF1], stretch);
    m_l_apf2_ = address(p[mLAPF2], stretch);
    m_r_apf2_ = address(p[mRAPF2], stretch);
    v_l_in_ = q15(p[vLIN]);
    v_r_in_ = q15(p[vRIN]);

    std::fill(delay_memory_.begin(), delay_memory_.end(), 0.0F);
}

float& PsxReverb::memory(const std::size_t offset) noexcept
{
    return delay_memory_[(offset + buffer_address_) & delay_memory_mask_];
}

} // namespace psx_reverb
