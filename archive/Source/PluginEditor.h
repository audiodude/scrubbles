#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <memory>

namespace scrubbles
{
class ScrubblesProcessor;

class ScrubblesEditor final : public juce::AudioProcessorEditor
{
public:
    explicit ScrubblesEditor (ScrubblesProcessor&);
    ~ScrubblesEditor() override;
    void resized() override;

private:
    class Content;
    std::unique_ptr<Content> content;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ScrubblesEditor)
};
} // namespace scrubbles
