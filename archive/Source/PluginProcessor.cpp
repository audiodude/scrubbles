#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>
#include <cstring>

namespace scrubbles
{
namespace
{
float readBounded (const std::atomic<float>* parameter, float low, float high, float fallback) noexcept
{
    const auto value = parameter->load (std::memory_order_relaxed);
    return std::isfinite (value) ? juce::jlimit (low, high, value) : fallback;
}
}

ScrubblesProcessor::ScrubblesProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "ScrubblesState", params::createLayout())
{
    seedValue = apvts.getRawParameterValue (params::seed);
    positionValue = apvts.getRawParameterValue (params::position);
    rateValue = apvts.getRawParameterValue (params::rate);
    depthValue = apvts.getRawParameterValue (params::depth);
    cutoffValue = apvts.getRawParameterValue (params::cutoff);
    resonanceValue = apvts.getRawParameterValue (params::resonance);
    runningValue = apvts.getRawParameterValue (params::running);
    reverseValue = apvts.getRawParameterValue (params::reverse);
    latchValue = apvts.getRawParameterValue (params::latch);
    static_assert (std::atomic<std::uint64_t>::is_always_lock_free,
                   "The audio-thread grab mailbox must be lock-free");
}

EngineSettings ScrubblesProcessor::readSettings() const noexcept
{
    EngineSettings settings;
    settings.seed = juce::roundToInt (readBounded (seedValue, 0.0f, 5.0f, 5.0f));
    settings.position = readBounded (positionValue, 0.0f, 1.0f, 0.0f);
    settings.rate = readBounded (rateValue, 0.02f, 8.0f, 0.25f);
    settings.depth = readBounded (depthValue, -8.0f, 8.0f, 4.0f);
    settings.cutoff = readBounded (cutoffValue, 20.0f, 18000.0f, 250.0f);
    settings.resonance = readBounded (resonanceValue, 0.0f, 1.0f, 0.25f);
    settings.running = readBounded (runningValue, 0.0f, 1.0f, 1.0f) >= 0.5f;
    settings.reverse = readBounded (reverseValue, 0.0f, 1.0f, 0.0f) >= 0.5f;
    settings.latch = readBounded (latchValue, 0.0f, 1.0f, 0.0f) >= 0.5f;
    return settings;
}

void ScrubblesProcessor::prepareToPlay (double sampleRate, int)
{
    keyboardState.reset();
    engine.prepare (sampleRate);
    consumedGrab = 0;
    engine.setSettings (readSettings());
}

void ScrubblesProcessor::releaseResources()
{
    releasePlayheadGrab();
}

bool ScrubblesProcessor::isBusesLayoutSupported (const BusesLayout& layout) const
{
    const auto output = layout.getMainOutputChannelSet();
    return layout.getMainInputChannelSet().isDisabled()
        && (output == juce::AudioChannelSet::mono() || output == juce::AudioChannelSet::stereo());
}

void ScrubblesProcessor::renderSamples (juce::AudioBuffer<float>& buffer, int begin, int end)
{
    auto* const* channels = buffer.getArrayOfWritePointers();
    const auto channelCount = buffer.getNumChannels();
    for (int sample = begin; sample < end; ++sample)
    {
        const auto value = engine.nextSample();
        for (int channel = 0; channel < channelCount; ++channel)
            channels[channel][sample] = value;
    }
}

void ScrubblesProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    const auto sampleCount = buffer.getNumSamples();
    if (sampleCount > 0)
        keyboardState.processNextMidiBuffer (midi, 0, sampleCount, true);
    engine.setSettings (readSettings());

    // Active state and position are a single coherent GUI-to-audio message.
    const auto grab = playheadGrab.load (std::memory_order_acquire);
    if (grab != consumedGrab)
    {
        const auto positionBits = static_cast<std::uint32_t> (grab);
        float grabPosition = 0.0f;
        std::memcpy (&grabPosition, &positionBits, sizeof (grabPosition));
        // Even a complete click between blocks must seek to its final coordinate.
        engine.setGrab (true, grabPosition);
        if ((grab & grabActiveMask) == 0)
            engine.setGrab (false, grabPosition);
        consumedGrab = grab;
    }

    int rendered = 0;
    for (const auto metadata : midi)
    {
        const auto eventSample = juce::jlimit (rendered, sampleCount, metadata.samplePosition);
        renderSamples (buffer, rendered, eventSample);
        rendered = eventSample;
        // This synth uses short MIDI messages only. Do not copy/allocate SysEx on the audio thread.
        if (metadata.numBytes > 0 && metadata.numBytes <= 3)
            engine.handleMidi (metadata.getMessage());
    }
    renderSamples (buffer, rendered, sampleCount);
    midi.clear();

    displayPosition.store (static_cast<float> (engine.readPosition()), std::memory_order_relaxed);
    displayCutoff.store (engine.currentCutoff(), std::memory_order_relaxed);
}

void ScrubblesProcessor::setPlayheadGrab (bool active, float position) noexcept
{
    position = std::isfinite (position) ? juce::jlimit (0.0f, 1.0f, position) : 0.0f;
    std::uint32_t bits = 0;
    std::memcpy (&bits, &position, sizeof (bits));
    auto previous = playheadGrab.load (std::memory_order_relaxed);
    for (;;)
    {
        if (! active && (previous & grabActiveMask) == 0)
            return;
        const auto sequence = (previous + grabSequenceIncrement) & ~(grabSequenceIncrement - 1);
        const auto next = sequence | static_cast<std::uint64_t> (bits) | (active ? grabActiveMask : 0);
        if (playheadGrab.compare_exchange_weak (previous, next, std::memory_order_release,
                                               std::memory_order_relaxed))
            return;
    }
}

void ScrubblesProcessor::releasePlayheadGrab() noexcept
{
    auto previous = playheadGrab.load (std::memory_order_relaxed);
    while ((previous & grabActiveMask) != 0)
    {
        const auto sequence = (previous + grabSequenceIncrement) & ~(grabSequenceIncrement - 1);
        const auto next = sequence | static_cast<std::uint32_t> (previous);
        if (playheadGrab.compare_exchange_weak (previous, next, std::memory_order_release,
                                               std::memory_order_relaxed))
            return;
    }
}

juce::AudioProcessorEditor* ScrubblesProcessor::createEditor()
{
    return new ScrubblesEditor (*this);
}

void ScrubblesProcessor::getStateInformation (juce::MemoryBlock& destination)
{
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary (*xml, destination);
}

void ScrubblesProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    const auto xml = getXmlFromBinary (data, sizeInBytes);
    if (xml == nullptr || ! xml->hasTagName (apvts.state.getType()))
        return;

    auto restored = juce::ValueTree::fromXml (*xml);
    if (! restored.isValid())
        return;

    // Presets are external input: reject non-finite values and snap to legal parameter ranges.
    for (auto child : restored)
    {
        const auto id = child.getProperty ("id").toString();
        if (auto* parameter = apvts.getParameter (id))
        {
            auto value = static_cast<float> (child.getProperty ("value"));
            if (! std::isfinite (value))
                value = parameter->convertFrom0to1 (parameter->getDefaultValue());
            const auto& range = parameter->getNormalisableRange();
            child.setProperty ("value", range.snapToLegalValue (juce::jlimit (range.start, range.end, value)), nullptr);
        }
    }
    apvts.replaceState (restored);
}
} // namespace scrubbles

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new scrubbles::ScrubblesProcessor();
}
