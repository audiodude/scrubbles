#pragma once

#include <juce_audio_basics/juce_audio_basics.h>

#include <array>
#include <cstddef>
#include <cstdint>

namespace scrubbles
{
enum class Seed
{
    Ramp,
    Triangle,
    Needle,
    Staircase,
    Gate,
    Landscape
};

// Unlike transport wrapping, this accepts the right endpoint for contour drawing.
float contourValue (Seed seed, double position) noexcept;

struct EngineSettings
{
    int seed = 5;
    float position = 0.0f;
    float rate = 0.25f;
    float depth = 4.0f;
    float cutoff = 250.0f;
    float resonance = 0.25f;
    bool running = true;
    bool reverse = false;
    bool latch = false;
};

// Owned exclusively by the audio thread. MIDI and settings may be applied between
// any two samples; neither method allocates or changes another note's transport.
class Engine
{
public:
    void prepare (double sampleRate) noexcept;
    void setSettings (const EngineSettings& settings) noexcept;
    void setGrab (bool active, float position) noexcept;
    void handleMidi (const juce::MidiMessage& message) noexcept;
    float nextSample() noexcept;

    double readPosition() const noexcept { return position_; }
    float currentCutoff() const noexcept { return cutoff_; }

private:
    static constexpr std::size_t midiVoiceCount = 24;
    static constexpr std::size_t latchVoiceCount = 3;
    static constexpr std::size_t voiceCount = midiVoiceCount + latchVoiceCount;

    struct Voice
    {
        double phase = 0.0;
        double phaseIncrement = 0.0;
        float velocity = 0.0f;
        float envelope = 0.0f;
        float integrator1 = 0.0f;
        float integrator2 = 0.0f;
        std::uint64_t age = 0;
        int note = 0;
        int channel = 0;
        bool active = false;
        bool keyDown = false;
        bool sustained = false;
        bool releasing = false;
    };

    Voice& allocateMidiVoice() noexcept;
    void startVoice (Voice& voice, int note, int channel, float velocity) noexcept;
    static void releaseVoice (Voice& voice) noexcept;
    void setLatch (bool enabled) noexcept;
    void releaseSustain (int channel) noexcept;
    void updateFilterCoefficients() noexcept;
    float cutoffTarget() const noexcept;

    EngineSettings settings_;
    std::array<Voice, voiceCount> voices_ {};
    std::array<bool, 16> sustain_ {};
    std::uint64_t nextAge_ = 0;
    double sampleRate_ = 44100.0;

    // This is travel + Position, modulo a cycle. Applying offset deltas directly
    // avoids an independent phase accumulator drifting away from the display.
    // While grabbed, offset changes are ignored; release thereby realigns the
    // implicit travel to the final position minus the *current* offset.
    double position_ = 0.0;
    double travelStep_ = 0.25 / 44100.0;
    bool grabbed_ = false;

    float attackStep_ = 1.0f / (0.006f * 44100.0f);
    float releaseMultiplier_ = 0.9988404f;
    float smoothingStep_ = 0.0056529f;
    float baseLogCutoff_ = 7.9657843f;
    float minimumLogCutoff_ = 4.3219281f;
    float maximumLogCutoff_ = 14.276565f;
    float smoothedLogCutoff_ = 7.9657843f;
    float smoothedResonance_ = 0.25f;
    float cutoff_ = 250.0f;
    float filterA1_ = 1.0f;
    float filterA2_ = 0.0f;
    float filterA3_ = 0.0f;
    int coefficientCountdown_ = 0;
};
}
