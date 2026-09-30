#include "../Source/PluginProcessor.h"
#include "../Source/Engine.h"
#include <juce_audio_utils/juce_audio_utils.h>
#include <cmath>
#include <iostream>
#include <stdexcept>

namespace {
void require(bool condition, const char* message)
{
    if (!condition) throw std::runtime_error(message);
}
bool near(double a, double b, double tolerance = 1.0e-5) { return std::abs(a - b) < tolerance; }
void advance(scrubbles::Engine& engine, int samples)
{
    for (int i = 0; i < samples; ++i)
        require(std::isfinite(engine.nextSample()), "Engine produced non-finite audio");
}
void transportChecks()
{
    scrubbles::Engine engine;
    engine.prepare(48000);
    scrubbles::EngineSettings settings;
    settings.position = 0.23f;
    settings.rate = 1.0f;
    engine.setSettings(settings);
    advance(engine, 4800);
    const auto moving = engine.readPosition();
    require(near(moving, 0.33, 0.0001), "Forward transport rate/offset incorrect");
    settings.running = false;
    engine.setSettings(settings);
    advance(engine, 4800);
    require(near(engine.readPosition(), moving), "Stop jumped or continued moving");
    settings.position += 0.15f;
    engine.setSettings(settings);
    advance(engine, 1);
    require(near(engine.readPosition(), moving + 0.15), "Stopped offset did not scrub");
    for (int seed = 0; seed < 6; ++seed) {
        settings.seed = seed;
        engine.setSettings(settings);
        advance(engine, 32);
        require(near(engine.readPosition(), moving + 0.15), "Seed change reset transport");
    }
    settings.running = true;
    settings.reverse = true;
    engine.setSettings(settings);
    advance(engine, 4800);
    require(near(engine.readPosition(), moving + 0.05, 0.0001), "Reverse transport incorrect");
    engine.setGrab(true, 0.72f);
    advance(engine, 4800);
    require(near(engine.readPosition(), 0.72), "Grab did not suspend motion");
    settings.position = 0.61f;
    engine.setSettings(settings);
    advance(engine, 32);
    require(near(engine.readPosition(), 0.72), "Position offset moved grabbed playhead");
    engine.setGrab(false, 0.72f);
    advance(engine, 1);
    require(near(engine.readPosition(), 0.72, 0.0001), "Release jumped with nonzero offset");
    advance(engine, 4800);
    require(near(engine.readPosition(), 0.62, 0.0001), "Release did not resume in reverse");
    settings.running = false;
    engine.setSettings(settings);
    engine.setGrab(true, 0.12f);
    advance(engine, 128);
    engine.setGrab(false, 0.12f);
    advance(engine, 4800);
    require(near(engine.readPosition(), 0.12), "Stopped grab release moved");
    std::cout << "PASS transport: freeze, offset, seed continuity, reverse, grab/release\n";
}
void audioChecks()
{
    for (double rate : { 44100.0, 48000.0, 96000.0 }) {
        scrubbles::Engine engine;
        engine.prepare(rate);
        scrubbles::EngineSettings settings;
        settings.running = false;
        engine.setSettings(settings);
        engine.handleMidi(juce::MidiMessage::noteOn(1, 60, (juce::uint8) 100));
        double energy = 0;
        for (int i = 0; i < (int) rate; ++i) {
            auto sample = engine.nextSample();
            require(std::isfinite(sample), "Non-finite note audio");
            energy += sample * sample;
        }
        require(energy / rate > 1.0e-6, "Note-on was silent");
        engine.handleMidi(juce::MidiMessage::noteOff(1, 60));
        advance(engine, (int) (rate * 2));
        double tail = 0;
        for (int i = 0; i < 2048; ++i) tail += std::abs(engine.nextSample());
        require(tail < 0.001, "Stopped contour prevented note release");
        settings.latch = true;
        engine.setSettings(settings);
        energy = 0;
        for (int i = 0; i < 4096; ++i) { auto sample = engine.nextSample(); energy += sample * sample; }
        require(energy > 0.001, "Latch was silent");
        settings.latch = false;
        engine.setSettings(settings);
        advance(engine, (int) (rate * 2));
        tail = 0;
        for (int i = 0; i < 2048; ++i) tail += std::abs(engine.nextSample());
        require(tail < 0.001, "Unlatch left stuck notes");
        engine.handleMidi(juce::MidiMessage::noteOn(2, 67, (juce::uint8) 100));
        engine.handleMidi(juce::MidiMessage::controllerEvent(2, 64, 127));
        engine.handleMidi(juce::MidiMessage::noteOff(2, 67));
        advance(engine, (int) rate);
        energy = 0;
        for (int i = 0; i < 2048; ++i) { auto sample = engine.nextSample(); energy += sample * sample; }
        require(energy > 0.001, "Sustain failed to retain released note");
        engine.handleMidi(juce::MidiMessage::controllerEvent(2, 64, 0));
        advance(engine, (int) (rate * 2));
        tail = 0;
        for (int i = 0; i < 2048; ++i) tail += std::abs(engine.nextSample());
        require(tail < 0.001, "Sustain release left stuck note");
        for (int seed = 0; seed < 6; ++seed) {
            settings.seed = seed;
            settings.latch = true;
            settings.running = true;
            settings.rate = 4;
            settings.depth = seed % 2 == 0 ? 8.0f : -8.0f;
            settings.cutoff = 18000;
            settings.resonance = 0.95f;
            engine.setSettings(settings);
            advance(engine, (int) rate);
            require(engine.currentCutoff() > 0 && engine.currentCutoff() < rate * 0.5, "Unsafe cutoff at frequency limits");
        }
    }
    std::cout << "PASS audio: audible notes, stopped note-off, latch release, six seeds at 44.1/48/96 kHz\n";
}
void processorChecks(const juce::File& renderFile)
{
    scrubbles::ScrubblesProcessor processor;
    processor.prepareToPlay(48000, 256);
    auto set = [&processor](const char* id, float value) {
        auto* parameter = processor.apvts.getParameter(id);
        require(parameter != nullptr, "Missing parameter");
        parameter->setValueNotifyingHost(parameter->convertTo0to1(value));
    };
    set("cutoff", 350);
    set("depth", 5);
    juce::AudioBuffer<float> rendered(2, 48000 * 4);
    rendered.clear();
    juce::MidiBuffer midi;
    for (int offset = 0; offset < rendered.getNumSamples(); offset += 256) {
        const auto count = juce::jmin(256, rendered.getNumSamples() - offset);
        juce::AudioBuffer<float> block(rendered.getArrayOfWritePointers(), 2, offset, count);
        midi.clear();
        if (offset == 0) {
            midi.addEvent(juce::MidiMessage::noteOn(1, 48, (juce::uint8) 90), 64);
            midi.addEvent(juce::MidiMessage::noteOn(1, 55, (juce::uint8) 80), 64);
            midi.addEvent(juce::MidiMessage::noteOn(1, 63, (juce::uint8) 80), 64);
        }
        if (offset == 96000) midi.addEvent(juce::MidiMessage::allNotesOff(1), 0);
        processor.processBlock(block, midi);
    }
    require(rendered.getMagnitude(0, 0, 64) == 0, "MIDI note began before sample offset");
    require(rendered.getRMSLevel(0, 64, 48000) > 0.001f, "Processor render silent");
    require(rendered.getMagnitude(0, 180000, 12000) < 0.00001f, "Processor MIDI release stuck");
    require(near(processor.apvts.getRawParameterValue("cutoff")->load(), 350, 0.01), "Modulation overwrote base cutoff");
    juce::MemoryBlock state;
    processor.getStateInformation(state);
    set("cutoff", 1200);
    processor.setStateInformation(state.getData(), (int) state.getSize());
    require(near(processor.apvts.getRawParameterValue("cutoff")->load(), 350, 0.01), "Patch state did not restore");
    set("running", 0);
    set("position", 0.37f);
    processor.setPlayheadGrab(true, 0.81f);
    processor.setPlayheadGrab(false, 0.81f);
    juce::AudioBuffer<float> gestureBlock(2, 256);
    midi.clear();
    processor.processBlock(gestureBlock, midi);
    require(near(processor.displayPosition.load(), 0.81), "Quick grab/release between blocks lost final seek");
    processor.processBlock(gestureBlock, midi);
    require(near(processor.displayPosition.load(), 0.81), "Inactive grab command reapplied offset");
    renderFile.deleteFile();
    juce::WavAudioFormat format;
    std::unique_ptr<juce::OutputStream> stream = renderFile.createOutputStream();
    require(stream != nullptr, "Cannot create render output");
    auto writer = format.createWriterFor(stream, juce::AudioFormatWriterOptions().withSampleRate(48000).withNumChannels(2).withBitsPerSample(24));
    require(writer != nullptr && writer->writeFromAudioSampleBuffer(rendered, 0, rendered.getNumSamples()), "Cannot write render");
    processor.releaseResources();
    std::cout << "PASS processor: sample-accurate MIDI, real-engine render, release, base cutoff isolation, state restore\n";
}
}
int main(int argc, char** argv)
{
    juce::ScopedJuceInitialiser_GUI initialise;
    try {
        if (argc > 2 && juce::String(argv[1]) == "--plugin") {
            juce::VST3PluginFormat format;
            juce::OwnedArray<juce::PluginDescription> descriptions;
            format.findAllTypesForFile(descriptions, juce::File(argv[2]).getFullPathName());
            require(descriptions.size() == 1, "VST3 discovery failed");
            juce::String error;
            auto plugin = format.createInstanceFromDescription(*descriptions[0], 48000, 256, error);
            require(plugin != nullptr, error.toRawUTF8());
            plugin->prepareToPlay(48000, 256);
            juce::AudioBuffer<float> audio(2, 256);
            juce::MidiBuffer midi;
            double energy = 0;
            for (int block = 0; block < 200; ++block) {
                audio.clear();
                midi.clear();
                if (block == 0) midi.addEvent(juce::MidiMessage::noteOn(1, 60, (juce::uint8) 100), 0);
                plugin->processBlock(audio, midi);
                energy += audio.getRMSLevel(0, 0, 256);
                require(std::isfinite(energy), "VST3 produced non-finite audio");
            }
            require(energy > 0.01, "Loaded VST3 produced no audio");
            std::unique_ptr<juce::AudioProcessorEditor> editor(plugin->createEditorAndMakeActive());
            require(editor != nullptr, "Loaded VST3 has no editor");
            editor.reset();
            plugin->releaseResources();
            std::cout << "PASS built VST3: discover, instantiate, MIDI render, native editor\n";
            return 0;
        }
        if (argc > 1 && juce::String(argv[1]) == "--ui") {
            scrubbles::ScrubblesProcessor processor;
            processor.prepareToPlay(48000, 256);
            std::unique_ptr<juce::AudioProcessorEditor> editor(processor.createEditor());
            editor->addToDesktop(juce::ComponentPeer::windowIsTemporary);
            editor->setVisible(true);
            juce::MessageManager::getInstance()->runDispatchLoopUntil(1000);
            const auto find = [](auto&& self, juce::Component& root, const juce::String& name) -> juce::Component* {
                if (root.getName() == name) return &root;
                for (auto* child : root.getChildren())
                    if (auto* result = self(self, *child, name)) return result;
                return nullptr;
            };
            auto component = [&](const char* name) {
                auto* result = find(find, *editor, name);
                require(result != nullptr, "UI control missing");
                return result;
            };
            juce::AudioBuffer<float> block(2, 256);
            juce::MidiBuffer midi;
            const auto render = [&] { midi.clear(); processor.processBlock(block, midi); };
            auto* latch = dynamic_cast<juce::Button*>(component("latch"));
            auto* run = dynamic_cast<juce::Button*>(component("running"));
            auto* seed = dynamic_cast<juce::ComboBox*>(component("seed"));
            auto* offset = dynamic_cast<juce::Slider*>(component("position"));
            require(latch && run && seed && offset, "UI control type mismatch");
            latch->triggerClick();
            juce::MessageManager::getInstance()->runDispatchLoopUntil(50);
            for (int i = 0; i < 100; ++i) render();
            require(block.getRMSLevel(0, 0, 256) > 0.001f, "UI latch failed to sound");
            run->triggerClick();
            juce::MessageManager::getInstance()->runDispatchLoopUntil(50);
            render();
            const auto stopped = processor.displayPosition.load();
            render();
            require(near(processor.displayPosition.load(), stopped), "UI Stop failed");
            offset->setValue(0.3, juce::sendNotificationSync);
            render();
            require(!near(processor.displayPosition.load(), stopped), "UI Position failed to scrub");
            const auto shifted = processor.displayPosition.load();
            for (int id = 1; id <= 6; ++id) {
                seed->setSelectedId(id, juce::sendNotificationSync);
                render();
                require(near(processor.displayPosition.load(), shifted), "UI seed selector reset position");
            }
            auto* contour = component("contour");
            const auto event = [&](float phase) {
                const juce::Point<float> point(28.0f + phase * (contour->getWidth() - 56.0f), 182.0f);
                return juce::MouseEvent(juce::Desktop::getInstance().getMainMouseSource(), point,
                    juce::ModifierKeys::leftButtonModifier, 1, 0, 0, 0, 0, contour, contour,
                    juce::Time::getCurrentTime(), point, juce::Time::getCurrentTime(), 1, true);
            };
            contour->mouseDown(event(0.22f));
            render();
            require(near(processor.displayPosition.load(), 0.22), "UI playhead grab failed");
            contour->mouseDrag(event(0.4f));
            render();
            require(near(processor.displayPosition.load(), 0.4), "UI playhead scrub failed");
            contour->mouseUp(event(0.4f));
            render();
            require(near(processor.displayPosition.load(), 0.4), "UI playhead release jumped");
            juce::MessageManager::getInstance()->runDispatchLoopUntil(100);
            std::cout << "PASS native UI: latch sound, stop, offset, seed changes, playhead grab/drag/release\n";
            juce::PNGImageFormat png;
            auto file = juce::File::getCurrentWorkingDirectory().getChildFile("build/scrubbles-ui.png");
            file.deleteFile();
            auto stream = file.createOutputStream();
            require(stream != nullptr && png.writeImageToStream(editor->createComponentSnapshot(editor->getLocalBounds(), true, 2.0f), *stream), "UI snapshot failed");
            std::cout << "UI snapshot: " << file.getFullPathName() << std::endl;
            return 0;
        }
        transportChecks();
        audioChecks();
        processorChecks(juce::File::getCurrentWorkingDirectory().getChildFile("scrubbles-render.wav"));
        std::cout << "All behavioral checks passed\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "FAIL: " << error.what() << '\n';
        return 1;
    }
}
