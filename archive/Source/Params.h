#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <array>
#include <cmath>

namespace scrubbles::params
{
inline constexpr auto seed = "seed";
inline constexpr auto position = "position";
inline constexpr auto rate = "rate";
inline constexpr auto depth = "depth";
inline constexpr auto cutoff = "cutoff";
inline constexpr auto resonance = "resonance";
inline constexpr auto running = "running";
inline constexpr auto reverse = "reverse";
inline constexpr auto latch = "latch";

inline constexpr std::array<const char*, 6> seedNames {
    "Ramp", "Triangle", "Needle", "Staircase", "Gate", "Landscape"
};

inline juce::String positionText (float value, int)
{
    return juce::String (value * 100.0f, 1) + "%";
}

inline juce::String rateText (float value, int)
{
    return juce::String (value, 2) + " Hz";
}

inline juce::String depthText (float value, int)
{
    return (value > 0.0f ? "+" : "") + juce::String (value, 1) + " oct";
}

inline juce::String cutoffText (float value, int)
{
    return value >= 1000.0f ? juce::String (value / 1000.0f, 2) + " kHz"
                            : juce::String (value, 0) + " Hz";
}

inline juce::String resonanceText (float value, int)
{
    return juce::String (value * 100.0f, 0) + "%";
}

inline float cutoffValue (const juce::String& text)
{
    return text.getFloatValue() * (text.containsIgnoreCase ("k") ? 1000.0f : 1.0f);
}

inline juce::AudioProcessorValueTreeState::ParameterLayout createLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    juce::StringArray names;
    for (const auto* name : seedNames)
        names.add (name);

    layout.add (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID { seed, 1 }, "Seed", names, 5));

    using Attributes = juce::AudioParameterFloatAttributes;
    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { position, 1 }, "Position", juce::NormalisableRange<float> { 0.0f, 1.0f }, 0.0f,
        Attributes().withStringFromValueFunction (positionText)
                    .withValueFromStringFunction ([] (const juce::String& text) { return text.getFloatValue() / 100.0f; })));

    auto rateRange = juce::NormalisableRange<float> { 0.02f, 8.0f, 0.001f };
    rateRange.setSkewForCentre (0.5f);
    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { rate, 1 }, "Rate", rateRange, 0.25f,
        Attributes().withStringFromValueFunction (rateText)
                    .withValueFromStringFunction ([] (const juce::String& text) { return text.getFloatValue(); })));

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { depth, 1 }, "Depth", juce::NormalisableRange<float> { -8.0f, 8.0f, 0.01f }, 4.0f,
        Attributes().withStringFromValueFunction (depthText)
                    .withValueFromStringFunction ([] (const juce::String& text) { return text.getFloatValue(); })));

    auto cutoffRange = juce::NormalisableRange<float> { 20.0f, 18000.0f, 1.0f };
    cutoffRange.setSkewForCentre (1000.0f);
    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { cutoff, 1 }, "Base cutoff", cutoffRange, 250.0f,
        Attributes().withStringFromValueFunction (cutoffText).withValueFromStringFunction (cutoffValue)));

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { resonance, 1 }, "Resonance", juce::NormalisableRange<float> { 0.0f, 1.0f, 0.001f }, 0.25f,
        Attributes().withStringFromValueFunction (resonanceText)
                    .withValueFromStringFunction ([] (const juce::String& text) { return text.getFloatValue() / 100.0f; })));

    layout.add (std::make_unique<juce::AudioParameterBool> (juce::ParameterID { running, 1 }, "Run", true));
    layout.add (std::make_unique<juce::AudioParameterBool> (juce::ParameterID { reverse, 1 }, "Reverse", false));
    layout.add (std::make_unique<juce::AudioParameterBool> (juce::ParameterID { latch, 1 }, "Latch chord", false));
    return layout;
}
} // namespace scrubbles::params
