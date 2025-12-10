#pragma once

#include <JuceHeader.h>

class AudioRecorder final : public juce::AudioIODeviceCallback
{
public:
    AudioRecorder (juce::AudioThumbnail& thumbnailToUpdate);
    ~AudioRecorder() override;

    void startRecording (const juce::File& file);
    void stop();
    bool isRecording() const;
    void audioDeviceAboutToStart (juce::AudioIODevice* device) override;
    void audioDeviceStopped() override;
    void audioDeviceIOCallbackWithContext (const float* const* inputChannelData, int numInputChannels,
                                           float* const* outputChannelData, int numOutputChannels,
                                           int numSamples, const juce::AudioIODeviceCallbackContext& context) override;
private:
    juce::AudioThumbnail& thumbnail;
    juce::TimeSliceThread backgroundThread { "Audio Recorder Thread" };
    std::unique_ptr<juce::AudioFormatWriter::ThreadedWriter> threadedWriter;
    double sampleRate = 0.0;
    juce::int64 nextSampleNum = 0;

    juce::CriticalSection writerLock;
    std::atomic<juce::AudioFormatWriter::ThreadedWriter*> activeWriter { nullptr };
};
