#include "Engine.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace scrubbles
{
namespace
{
constexpr double pi = 3.14159265358979323846;
constexpr double pulseWidth = 0.42;
constexpr int coefficientInterval = 8;

float finiteClamp (float value, float low, float high, float fallback) noexcept
{
    return std::isfinite (value) ? std::clamp (value, low, high) : fallback;
}

double wrap (double position) noexcept
{
    return position - std::floor (position);
}

float polyBlep (double phase, double increment) noexcept
{
    if (phase < increment)
    {
        const auto t = phase / increment;
        return static_cast<float> (2.0 * t - t * t - 1.0);
    }

    if (phase > 1.0 - increment)
    {
        const auto t = (phase - 1.0) / increment;
        return static_cast<float> (t * t + 2.0 * t + 1.0);
    }

    return 0.0f;
}
}

float contourValue (Seed seed, double position) noexcept
{
    const auto p = std::isfinite (position) ? std::clamp (position, 0.0, 1.0) : 0.0;

    switch (seed)
    {
        case Seed::Ramp:
            return static_cast<float> (p);

        case Seed::Triangle:
            return static_cast<float> (1.0 - std::abs (2.0 * p - 1.0));

        case Seed::Needle:
            if (p < 0.35 || p >= 0.50)
                return 0.0f;
            if (p < 0.40)
                return static_cast<float> ((p - 0.35) / 0.05);
            return static_cast<float> ((0.50 - p) / 0.10);

        case Seed::Staircase:
            if (p < 0.25)
                return 0.0f;
            if (p < 0.50)
                return 1.0f / 3.0f;
            if (p < 0.75)
                return 2.0f / 3.0f;
            return 1.0f;

        case Seed::Gate:
            return p >= 0.25 && p < 0.75 ? 1.0f : 0.0f;

        case Seed::Landscape:
        default:
            // (0,.12), (.16,.12), (.22,1), (.28,.80), (.36,.80),
            // (.36,.58), (.46,.58), (.46,.36), (.58,.36), (1,.12).
            // Duplicate positions are right-continuous downward steps.
            if (p < 0.16)
                return 0.12f;
            if (p < 0.22)
                return static_cast<float> (0.12 + 0.88 * (p - 0.16) / 0.06);
            if (p < 0.28)
                return static_cast<float> (1.0 - 0.20 * (p - 0.22) / 0.06);
            if (p < 0.36)
                return 0.80f;
            if (p < 0.46)
                return 0.58f;
            if (p < 0.58)
                return 0.36f;
            return static_cast<float> (0.36 - 0.24 * (p - 0.58) / 0.42);
    }
}

void Engine::prepare (double sampleRate) noexcept
{
    sampleRate_ = std::isfinite (sampleRate) && sampleRate >= 1000.0 ? sampleRate : 44100.0;
    voices_.fill (Voice {});
    sustain_.fill (false);
    nextAge_ = 0;
    grabbed_ = false;
    position_ = wrap (static_cast<double> (settings_.position));
    travelStep_ = static_cast<double> (settings_.rate) / sampleRate_
                  * (settings_.reverse ? -1.0 : 1.0);

    attackStep_ = static_cast<float> (1.0 / (0.006 * sampleRate_));
    // Release reaches -80 dB after 180 ms, regardless of traversal state.
    releaseMultiplier_ = static_cast<float> (std::exp (std::log (0.0001) / (0.180 * sampleRate_)));
    smoothingStep_ = static_cast<float> (-std::expm1 (-1.0 / (0.004 * sampleRate_)));
    baseLogCutoff_ = std::log2 (settings_.cutoff);
    minimumLogCutoff_ = std::log2 (20.0f);
    maximumLogCutoff_ = std::log2 (static_cast<float> (std::min (20000.0, sampleRate_ * 0.45)));
    smoothedLogCutoff_ = cutoffTarget();
    smoothedResonance_ = settings_.resonance;
    updateFilterCoefficients();
    coefficientCountdown_ = coefficientInterval;

    if (settings_.latch)
        setLatch (true);
}

void Engine::setSettings (const EngineSettings& requested) noexcept
{
    auto next = requested;
    next.seed = std::clamp (next.seed, 0, 5);
    next.position = finiteClamp (next.position, 0.0f, 1.0f, settings_.position);
    next.rate = finiteClamp (next.rate, 0.0f, 100.0f, settings_.rate);
    next.depth = finiteClamp (next.depth, -16.0f, 16.0f, settings_.depth);
    next.cutoff = finiteClamp (next.cutoff, 20.0f, 20000.0f, settings_.cutoff);
    next.resonance = finiteClamp (next.resonance, 0.0f, 1.0f, settings_.resonance);

    if (! grabbed_ && next.position != settings_.position)
        position_ = wrap (position_ + static_cast<double> (next.position)
                          - static_cast<double> (settings_.position));

    const bool latchChanged = next.latch != settings_.latch;
    if (next.cutoff != settings_.cutoff)
        baseLogCutoff_ = std::log2 (next.cutoff);

    settings_ = next;
    travelStep_ = static_cast<double> (settings_.rate) / sampleRate_
                  * (settings_.reverse ? -1.0 : 1.0);

    if (latchChanged)
        setLatch (settings_.latch);
}

void Engine::setGrab (bool active, float position) noexcept
{
    // Ignore stale coordinates in the GUI mailbox when no gesture is active.
    // Releasing consumes the final position only on the active -> inactive edge.
    if (active || grabbed_)
        position_ = static_cast<double> (finiteClamp (position, 0.0f, 1.0f,
                                                     static_cast<float> (position_)));

    grabbed_ = active;
}

Engine::Voice& Engine::allocateMidiVoice() noexcept
{
    Voice* quietestReleased = nullptr;
    Voice* oldestHeld = &voices_.front();
    auto quietestLevel = std::numeric_limits<float>::max();

    for (std::size_t i = 0; i < midiVoiceCount; ++i)
    {
        auto& voice = voices_[i];
        if (! voice.active)
            return voice;

        if (voice.releasing && voice.envelope * voice.velocity < quietestLevel)
        {
            quietestLevel = voice.envelope * voice.velocity;
            quietestReleased = &voice;
        }

        if (voice.age < oldestHeld->age)
            oldestHeld = &voice;
    }

    return quietestReleased != nullptr ? *quietestReleased : *oldestHeld;
}

void Engine::startVoice (Voice& voice, int note, int channel, float velocity) noexcept
{
    voice = Voice {};
    voice.note = note;
    voice.channel = channel;
    voice.velocity = finiteClamp (velocity, 0.0f, 1.0f, 0.0f);
    voice.phaseIncrement = std::min (440.0 * std::exp2 ((static_cast<double> (note) - 69.0) / 12.0)
                                    / sampleRate_, 0.45);
    voice.age = nextAge_++;
    voice.active = true;
    voice.keyDown = true;
}

void Engine::releaseVoice (Voice& voice) noexcept
{
    voice.keyDown = false;
    voice.sustained = false;
    voice.releasing = true;
}

void Engine::setLatch (bool enabled) noexcept
{
    constexpr std::array<int, latchVoiceCount> chord { 48, 51, 55 };
    for (std::size_t i = 0; i < latchVoiceCount; ++i)
    {
        auto& voice = voices_[midiVoiceCount + i];
        if (enabled)
            startVoice (voice, chord[i], 0, 0.72f);
        else
            releaseVoice (voice);
    }
}

void Engine::releaseSustain (int channel) noexcept
{
    sustain_[static_cast<std::size_t> (channel - 1)] = false;
    for (std::size_t i = 0; i < midiVoiceCount; ++i)
    {
        auto& voice = voices_[i];
        if (voice.active && voice.channel == channel && voice.sustained)
            releaseVoice (voice);
    }
}

void Engine::handleMidi (const juce::MidiMessage& message) noexcept
{
    const auto channel = message.getChannel();
    if (channel < 1 || channel > 16)
        return;

    if (message.isNoteOn())
    {
        const auto note = message.getNoteNumber();
        // Repeated note-ons cannot strand an older key-down instance. Existing
        // pedal-held instances still belong to the pedal, not to this new key.
        for (std::size_t i = 0; i < midiVoiceCount; ++i)
        {
            auto& voice = voices_[i];
            if (voice.active && voice.keyDown && voice.channel == channel && voice.note == note)
                releaseVoice (voice);
        }

        startVoice (allocateMidiVoice(), note, channel, message.getFloatVelocity());
        return;
    }

    if (message.isNoteOff())
    {
        for (std::size_t i = 0; i < midiVoiceCount; ++i)
        {
            auto& voice = voices_[i];
            if (voice.active && voice.keyDown && voice.channel == channel
                && voice.note == message.getNoteNumber())
            {
                voice.keyDown = false;
                if (sustain_[static_cast<std::size_t> (channel - 1)])
                    voice.sustained = true;
                else
                    releaseVoice (voice);
            }
        }
        return;
    }

    if (! message.isController())
        return;

    switch (message.getControllerNumber())
    {
        case 64:
            if (message.getControllerValue() >= 64)
                sustain_[static_cast<std::size_t> (channel - 1)] = true;
            else
                releaseSustain (channel);
            break;

        case 120: // All sound off: immediate channel panic, including release tails.
        case 123: // All notes off: release even if the sustain pedal was held.
        {
            const bool immediate = message.getControllerNumber() == 120;
            sustain_[static_cast<std::size_t> (channel - 1)] = false;
            for (std::size_t i = 0; i < midiVoiceCount; ++i)
            {
                auto& voice = voices_[i];
                if (voice.active && voice.channel == channel)
                {
                    if (immediate)
                        voice = Voice {};
                    else
                        releaseVoice (voice);
                }
            }
            break;
        }

        case 121: // Reset controllers releases pedal-held, but not physically held, notes.
            releaseSustain (channel);
            break;

        default:
            break;
    }
}

float Engine::cutoffTarget() const noexcept
{
    // Clamp only the derived frequency, never the user's base-cutoff parameter.
    return std::clamp (baseLogCutoff_
                       + settings_.depth * contourValue (static_cast<Seed> (settings_.seed), position_),
                       minimumLogCutoff_, maximumLogCutoff_);
}

void Engine::updateFilterCoefficients() noexcept
{
    cutoff_ = std::exp2 (smoothedLogCutoff_);
    const auto g = static_cast<float> (std::tan (pi * static_cast<double> (cutoff_) / sampleRate_));
    const auto damping = 2.0f - 1.85f * smoothedResonance_; // Q = 0.5 .. 6.67.
    filterA1_ = 1.0f / (1.0f + g * (g + damping));
    filterA2_ = g * filterA1_;
    filterA3_ = g * filterA2_;
}

float Engine::nextSample() noexcept
{
    smoothedLogCutoff_ += smoothingStep_ * (cutoffTarget() - smoothedLogCutoff_);
    smoothedResonance_ += smoothingStep_ * (settings_.resonance - smoothedResonance_);
    if (coefficientCountdown_ == 0)
    {
        updateFilterCoefficients();
        coefficientCountdown_ = coefficientInterval;
    }
    --coefficientCountdown_;

    float sum = 0.0f;
    for (auto& voice : voices_)
    {
        if (! voice.active)
            continue;

        if (voice.releasing)
        {
            voice.envelope *= releaseMultiplier_;
            if (voice.envelope <= 0.0001f)
            {
                voice = Voice {};
                continue;
            }
        }
        else
        {
            voice.envelope = std::min (1.0f, voice.envelope + attackStep_);
        }

        const auto edgeCorrection = polyBlep (voice.phase, voice.phaseIncrement);
        const auto saw = static_cast<float> (2.0 * voice.phase - 1.0) - edgeCorrection;
        auto pulsePhase = voice.phase + (1.0 - pulseWidth);
        if (pulsePhase >= 1.0)
            pulsePhase -= 1.0;
        const auto pulse = (voice.phase < pulseWidth ? 1.0f : -1.0f) + edgeCorrection
                           - polyBlep (pulsePhase, voice.phaseIncrement)
                           - static_cast<float> (2.0 * pulseWidth - 1.0);
        const auto input = 0.65f * saw - 0.35f * pulse;

        voice.phase += voice.phaseIncrement;
        if (voice.phase >= 1.0)
            voice.phase -= 1.0;

        // Topology-preserving state-variable lowpass. Coefficients are global;
        // only the two integrator states and envelope belong to each voice.
        const auto v3 = input - voice.integrator2;
        const auto v1 = filterA1_ * voice.integrator1 + filterA2_ * v3;
        const auto v2 = voice.integrator2 + filterA2_ * voice.integrator1 + filterA3_ * v3;
        voice.integrator1 = 2.0f * v1 - voice.integrator1;
        voice.integrator2 = 2.0f * v2 - voice.integrator2;
        sum += v2 * voice.envelope * voice.velocity;
    }

    if (settings_.running && ! grabbed_)
        position_ = wrap (position_ + travelStep_);

    // Fixed headroom plus a bounded, inexpensive soft saturator for dense chords
    // at maximum resonance; no per-voice transcendental functions in this loop.
    const auto output = 0.12f * sum;
    return 0.95f * output / (1.0f + std::abs (output));
}
}
