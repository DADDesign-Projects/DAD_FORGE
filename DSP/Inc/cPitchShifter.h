//==================================================================================
//==================================================================================
// File: cPitchShifter.h
// Description: High-quality +1 octave pitch shifter for shimmer reverb
//
// Copyright (c) 2026 DadDSP.
//==================================================================================
//==================================================================================

#pragma once

//**********************************************************************************
// Includes
//**********************************************************************************
#include <cmath>
#include <algorithm>

//**********************************************************************************
// Namespace
//**********************************************************************************
namespace DadDSP {

//**********************************************************************************
// Class: cPitchShifter
// Description: Professional octave-up pitch shifter using 4 grains with temporal
//              dispersion and optimal windowing
//**********************************************************************************
class cPitchShifter
{
public:
    // -----------------------------------------------------------------------------
    // Public methods
    // -----------------------------------------------------------------------------

	// -----------------------------------------------------------------------------
	//
	cPitchShifter() = default;

	// -----------------------------------------------------------------------------
	// Initialize the pitch shifter with sample rate
    void Initialize(uint32_t sampleRate)
    {
        m_SampleRate = sampleRate;

        std::fill(std::begin(m_Buffer), std::end(m_Buffer), 0.0f);

        GenerateWindow();  // Generate optimal window for 4 grains

        m_WritePos = 0;

        // Initialize 4 grains with temporal dispersion
        for (uint16_t i = 0; i < NUM_GRAINS; ++i)
        {
            m_Grains[i].active = (i == 0);  // Only first grain starts active
            m_Grains[i].phase = 0;
            m_Grains[i].startPos = 0;
            m_Grains[i].startDelay = i * HOP_SIZE;
        }

        m_SampleCounter = 0;

        // Filter states initialization
        m_DcBlockX1 = 0.0f;
        m_DcBlockY1 = 0.0f;
        m_PreEmphZ1 = 0.0f;

        // Anti-aliasing low-pass filter
        m_LpfState = 0.0f;
        m_LpfCoeff = 0.7f;  // Adjustable as needed
    }

	// -----------------------------------------------------------------------------
    // Process one audio sample
    inline float Process(float input)
    {
        // ─────────────────────────────────────────────────────────────────────────────
    	// Step 1: Pre-emphasis to reduce low-frequency artifacts
        float preEmph = input - m_PreEmphZ1 * 0.97f;
        m_PreEmphZ1 = input;

        // ─────────────────────────────────────────────────────────────────────────────
        // Step 2: Write to circular buffer
        m_Buffer[m_WritePos] = preEmph;
        m_WritePos = (m_WritePos + 1) & BUFFER_MASK;


        // ─────────────────────────────────────────────────────────────────────────────
        // Step 3: Progressive grain activation
        for (uint16_t i = 0; i < NUM_GRAINS; ++i)
        {
            if (!m_Grains[i].active && m_SampleCounter >= m_Grains[i].startDelay)
            {
                m_Grains[i].active = true;
                m_Grains[i].phase = 0;
                m_Grains[i].startPos = m_WritePos;
            }
        }

        m_SampleCounter++;

        // ─────────────────────────────────────────────────────────────────────────────
        // Step 4: Process all active grains
        float output = 0.0f;

        for (uint16_t i = 0; i < NUM_GRAINS; ++i)
        {
            if (m_Grains[i].active)
            {
                output += ProcessGrain(m_Grains[i]);
            }
        }

        // ─────────────────────────────────────────────────────────────────────────────
        // Step 5: Normalization for 4 grains with 75% overlap
        // Theoretically: sum of windows² = constant
        output *= 0.35f;  // Adjusted empirically

        // ─────────────────────────────────────────────────────────────────────────────
        // Step 6: Anti-aliasing low-pass filter
        m_LpfState = m_LpfState * m_LpfCoeff + output * (1.0f - m_LpfCoeff);
        output = m_LpfState;

        // ─────────────────────────────────────────────────────────────────────────────
        // Step 7: DC blocker
        float dcBlocked = output - m_DcBlockX1 + 0.998f * m_DcBlockY1;
        m_DcBlockX1 = output;
        m_DcBlockY1 = dcBlocked;

        // ─────────────────────────────────────────────────────────────────────────────
        // Step 8: Soft saturation (prevents hard clipping)
        return SoftSaturate(dcBlocked);
    }

	// -----------------------------------------------------------------------------
    // Set interpolation quality mode
    void SetQuality(bool highQuality)
    {
        m_UseHermite = highQuality;
    }

	// -----------------------------------------------------------------------------
    // Adjust anti-aliasing filter brightness (0.5 = darker, 0.9 = brighter)
    void SetBrightness(float brightness)
    {
        m_LpfCoeff = std::max(0.3f, std::min(0.95f, brightness));
    }

private:
    // ---------------------------------------------------------------------------------
    // Private structures
    // ---------------------------------------------------------------------------------

    struct Grain
    {
        bool active;        // Grain activation state
        uint32_t phase;     // Current phase within grain
        uint32_t startPos;  // Starting position in buffer
        uint32_t startDelay;// Delay before activation
    };

    // =============================================================================
    // Static constants
    // =============================================================================

    static constexpr uint16_t BUFFER_SIZE = 8192;   // Circular buffer size (power of 2)
    static constexpr uint16_t BUFFER_MASK = BUFFER_SIZE - 1;  // For fast modulo
    static constexpr uint16_t GRAIN_SIZE = 1536;    // Approximately 32ms @ 48kHz
    static constexpr uint16_t HOP_SIZE = 384;       // 75% overlap between grains
    static constexpr uint16_t NUM_GRAINS = 4;       // Number of parallel grains

    // ---------------------------------------------------------------------------------
    // Private methods
    // ---------------------------------------------------------------------------------

	// -----------------------------------------------------------------------------
    // Process a single grain
    float ProcessGrain(Grain& grain)
    {
        // Read position with 2x pitch shift
        float readOffset = static_cast<float>(grain.phase) * 2.0f;
        uint32_t readPos = (grain.startPos + static_cast<uint32_t>(readOffset)) & BUFFER_MASK;

        // Quality interpolation
        float sample = m_UseHermite ?
            HermiteInterpolate(readPos, readOffset) :
            LinearInterpolate(readPos, readOffset);

        // Apply windowing function
        float window = m_Window[grain.phase];
        float output = sample * window;

        grain.phase++;

        // Restart grain with micro-variation to avoid phasing
        if (grain.phase >= GRAIN_SIZE)
        {
            grain.phase = 0;
            // Add small deterministic random variation
            uint32_t offset = 32 + ((grain.startPos * 7) & 63);  // 32-95 samples
            grain.startPos = (m_WritePos + offset) & BUFFER_MASK;
        }

        return output;
    }

	// -----------------------------------------------------------------------------
    // Fast linear interpolation
    float LinearInterpolate(uint32_t basePos, float offset)
    {
        float frac = offset - static_cast<uint32_t>(offset);
        uint32_t idx1 = basePos & BUFFER_MASK;
        uint32_t idx2 = (basePos + 1) & BUFFER_MASK;

        return m_Buffer[idx1] + frac * (m_Buffer[idx2] - m_Buffer[idx1]);
    }

	// -----------------------------------------------------------------------------
    // Hermite interpolation (better quality/CPU compromise)
    float HermiteInterpolate(uint32_t basePos, float offset)
    {
        float frac = offset - static_cast<uint32_t>(offset);

        uint32_t idx0 = (basePos - 1) & BUFFER_MASK;
        uint32_t idx1 = basePos & BUFFER_MASK;
        uint32_t idx2 = (basePos + 1) & BUFFER_MASK;
        uint32_t idx3 = (basePos + 2) & BUFFER_MASK;

        float y0 = m_Buffer[idx0];
        float y1 = m_Buffer[idx1];
        float y2 = m_Buffer[idx2];
        float y3 = m_Buffer[idx3];

        // 4-point Hermite interpolation
        float c0 = y1;
        float c1 = 0.5f * (y2 - y0);
        float c2 = y0 - 2.5f * y1 + 2.0f * y2 - 0.5f * y3;
        float c3 = 0.5f * (y3 - y0) + 1.5f * (y1 - y2);

        return ((c3 * frac + c2) * frac + c1) * frac + c0;
    }

	// -----------------------------------------------------------------------------
    // Generate optimal window for 4 grains
    void GenerateWindow()
    {
        const float pi = 3.14159265358979323846f;
        for (uint16_t i = 0; i < GRAIN_SIZE; ++i)
        {
            float t = static_cast<float>(i) / static_cast<float>(GRAIN_SIZE);

            // Hann window with smoother transition
            float hann = 0.5f * (1.0f - cosf(2.0f * pi * t));

            // Slight compensation for 75% overlap
            m_Window[i] = powf(hann, 0.85f);
        }
    }

	// -----------------------------------------------------------------------------
    // Soft tube-like saturation (more musical)
    float SoftSaturate(float x)
    {
        // Asymmetric saturation (like a tube)
        if (x > 0.7f)
        {
            float excess = x - 0.7f;
            return 0.7f + 0.3f * tanhf(excess * 2.5f);
        }
        else if (x < -0.7f)
        {
            float excess = x + 0.7f;
            return -0.7f + 0.3f * tanhf(excess * 2.5f);
        }
        return x;
    }

    // =============================================================================
    // Private member variables
    // =============================================================================

    float     m_Buffer[BUFFER_SIZE];     // Circular delay buffer
    float     m_Window[GRAIN_SIZE];      // Grain window function

    Grain     m_Grains[NUM_GRAINS];      // Array of grains
    uint16_t  m_WritePos;                // Current write position in buffer
    uint16_t  m_SampleCounter;           // Sample counter for grain activation

    // Filter states
    float     m_PreEmphZ1;               // Pre-emphasis filter state
    float     m_DcBlockX1;               // DC blocker input state
    float     m_DcBlockY1;               // DC blocker output state
    float     m_LpfState;                // Low-pass filter state
    float     m_LpfCoeff;                // Low-pass filter coefficient

    // Configuration options
    bool      m_UseHermite = true;       // Use Hermite interpolation when true

    uint32_t  m_SampleRate;              // System sample rate
};

} // namespace DadDSP

//***End of file**************************************************************
