#pragma once

#include "dsp/parameters.hpp"

#include <cstddef>
#include <vector>

namespace psx_reverb {

struct StereoFrame {
    float left;
    float right;
};

class PsxReverb {
public:
    void prepare(float sample_rate);
    void reset() noexcept;
    void set_parameters(const Parameters& parameters) noexcept;

    [[nodiscard]] StereoFrame process_sample(float left, float right) noexcept;
    void process(
        const float* left_in,
        const float* right_in,
        float* left_out,
        float* right_out,
        std::size_t frames) noexcept;

private:
    void load_preset(Preset preset) noexcept;
    [[nodiscard]] float& memory(std::size_t offset) noexcept;

    Parameters parameters_ {};
    float sample_rate_ = 0.0F;

    std::vector<float> delay_memory_;
    std::size_t delay_memory_mask_ = 0;
    std::size_t buffer_address_ = 0;

    float wet_ = 1.0F;
    float dry_ = 1.0F;
    float master_ = 1.0F;
    float wet_target_ = 1.0F;
    float dry_target_ = 1.0F;
    float master_target_ = 1.0F;

    std::size_t d_apf1_ = 0;
    std::size_t d_apf2_ = 0;
    float v_iir_ = 0.0F;
    float v_comb1_ = 0.0F;
    float v_comb2_ = 0.0F;
    float v_comb3_ = 0.0F;
    float v_comb4_ = 0.0F;
    float v_wall_ = 0.0F;
    float v_apf1_ = 0.0F;
    float v_apf2_ = 0.0F;
    std::size_t m_l_same_ = 0;
    std::size_t m_r_same_ = 0;
    std::size_t m_l_comb1_ = 0;
    std::size_t m_r_comb1_ = 0;
    std::size_t m_l_comb2_ = 0;
    std::size_t m_r_comb2_ = 0;
    std::size_t d_l_same_ = 0;
    std::size_t d_r_same_ = 0;
    std::size_t m_l_diff_ = 0;
    std::size_t m_r_diff_ = 0;
    std::size_t m_l_comb3_ = 0;
    std::size_t m_r_comb3_ = 0;
    std::size_t m_l_comb4_ = 0;
    std::size_t m_r_comb4_ = 0;
    std::size_t d_l_diff_ = 0;
    std::size_t d_r_diff_ = 0;
    std::size_t m_l_apf1_ = 0;
    std::size_t m_r_apf1_ = 0;
    std::size_t m_l_apf2_ = 0;
    std::size_t m_r_apf2_ = 0;
    float v_l_in_ = 0.0F;
    float v_r_in_ = 0.0F;
};

} // namespace psx_reverb
