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
    
    std::unique_ptr<AudioRecorder> recorder;
    RecordingThumbnail recordingThumbnail;
    
    juce::AudioFormatManager formatManager;
    juce::AudioTransportSource transportSource;
    std::unique_ptr<juce::ResamplingAudioSource> resamplingSource;
    juce::File currentRecordingFile;

    TransportState state;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};