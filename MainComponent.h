#pragma once

#include <JuceHeader.h>
#include "AudioRecorder.h"
#include "RecordingThumbnail.h"

class MainComponent   : public juce::AudioAppComponent,
                        public juce::ChangeListener
{
public:
    MainComponent();
    ~MainComponent() override;

    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    void paint (juce::Graphics& g) override;
    void resized() override;

private:
    void changeListenerCallback (juce::ChangeBroadcaster* source) override;

    void startRecording();
    void stopRecording();

    void startPlaying();
    void stopPlaying();
    
    enum TransportState
    {
        Stopped,
        Starting,
        Playing,
        Stopping
    };

    void transportStateChanged (TransportState newState);

    juce::TextButton recordButton   { "Record" };
    juce::TextButton playButton     { "Play" };

    juce::Slider speedSlider;
    juce::Label speedLabel          { {}, "Speed:" };

    juce::Slider delayFeedbackSlider;
    juce::Label delayFeedbackLabel  { {}, "Feedback:" };

    juce::Slider delayWetDrySlider;
    juce::Label delayWetDryLabel    { {}, "Delay Wet/Dry:" };

    juce::Slider reverbRoomSizeSlider;
    juce::Label reverbRoomSizeLabel { {}, "Room Size:" };

    juce::Slider reverbWetDrySlider;
    juce::Label reverbWetDryLabel   { {}, "Reverb Wet/Dry:" };

    juce::Slider reverbDampingSlider;
    juce::Label reverbDampingLabel  { {}, "Reverb Damping:" };
    juce::Slider reverbWidthSlider;
    juce::Label reverbWidthLabel    { {}, "Reverb Width:" };

    juce::Slider flangerRateSlider;
    juce::Label flangerRateLabel    { {}, "Flanger Rate:" };
    juce::Slider flangerDepthSlider;
    juce::Label flangerDepthLabel   { {}, "Flanger Depth:" };
    juce::Slider flangerFeedbackSlider;
    juce::Label flangerFeedbackLabel{ {}, "Flanger F/back:" };
    juce::Slider flangerMixSlider;
    juce::Label flangerMixLabel     { {}, "Flanger Mix:" };

    juce::Slider pitchShiftSlider;
    juce::Label pitchShiftLabel     { {}, "Pitch Shift:" };
    
    std::unique_ptr<juce::dsp::DelayLine<float>> delayLine;
    std::unique_ptr<juce::dsp::Reverb> reverb;
    std::unique_ptr<juce::dsp::Chorus<float>> flanger;

    double currentSampleRate = 0.0;

    std::unique_ptr<AudioRecorder> recorder;
    RecordingThumbnail recordingThumbnail;
    
    juce::AudioFormatManager formatManager;
    juce::AudioTransportSource transportSource;
    std::unique_ptr<juce::ResamplingAudioSource> resamplingSource;
    juce::File currentRecordingFile;

    TransportState state;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};