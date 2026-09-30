#pragma once

#include <juce_audio_utils/juce_audio_utils.h>
#include "Engine.h"
#include "Params.h"
#include <atomic>
#include <cstdint>

namespace scrubbles
{
class ScrubblesProcessor final : public juce::AudioProcessor
{
public:
    ScrubblesProcessor();
    ~ScrubblesProcessor() override = default;

    void prepareToPlay (double sampleRate, int maximumExpectedSamplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout&) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using juce::AudioProcessor::processBlock;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "Scrubbles"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.5; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}
    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int sizeInBytes) override;

    void setPlayheadGrab (bool active, float position) noexcept;
    void releasePlayheadGrab() noexcept;

    juce::AudioProcessorValueTreeState apvts;
    juce::MidiKeyboardState keyboardState;
    std::atomic<float> displayPosition { 0.0f };
    std::atomic<float> displayCutoff { 250.0f };

private:
    EngineSettings readSettings() const noexcept;
    void renderSamples (juce::AudioBuffer<float>&, int begin, int end);

    Engine engine;
    std::uint64_t consumedGrab = 0;
    static constexpr std::uint64_t grabSequenceIncrement = std::uint64_t { 1 } << 33;
    std::atomic<std::uint64_t> playheadGrab { 0 };
    static constexpr std::uint64_t grabActiveMask = std::uint64_t { 1 } << 32;

    std::atomic<float>* seedValue = nullptr;
    std::atomic<float>* positionValue = nullptr;
    std::atomic<float>* rateValue = nullptr;
    std::atomic<float>* depthValue = nullptr;
    std::atomic<float>* cutoffValue = nullptr;
    std::atomic<float>* resonanceValue = nullptr;
    std::atomic<float>* runningValue = nullptr;
    std::atomic<float>* reverseValue = nullptr;
    std::atomic<float>* latchValue = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ScrubblesProcessor)
};
} // namespace scrubbles
