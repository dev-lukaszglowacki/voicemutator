#pragma once

#include <JuceHeader.h>

class RecordingThumbnail final : public juce::Component,
                                 private juce::ChangeListener
{
public:
    RecordingThumbnail();
    ~RecordingThumbnail() override;

    juce::AudioThumbnail& getAudioThumbnail();
    void setDisplayFullThumbnail (bool displayFull);
    void paint (juce::Graphics& g) override;

private:
    juce::AudioFormatManager formatManager;
    juce::AudioThumbnailCache thumbnailCache  { 10 };
    juce::AudioThumbnail thumbnail            { 512, formatManager, thumbnailCache };

    bool displayFullThumb = false;

    void changeListenerCallback (juce::ChangeBroadcaster* source) override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RecordingThumbnail)
};
