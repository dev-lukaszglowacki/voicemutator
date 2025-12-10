#include "MainComponent.h"

MainComponent::MainComponent() : state(Stopped)
{
    addAndMakeVisible (recordButton);
    recordButton.setColour (juce::TextButton::buttonColourId, juce::Colour (0xffff5c5c));
    recordButton.setColour (juce::TextButton::textColourOnId, juce::Colours::black);
    recordButton.onClick = [this] {
        if (recorder->isRecording())
            stopRecording();
        else
            startRecording();
    };

    addAndMakeVisible (playButton);
    playButton.onClick = [this] {
        if (transportSource.isPlaying())
            stopPlaying();
        else
            startPlaying();
    };
    playButton.setEnabled(false);

    addAndMakeVisible (speedSlider);
    speedSlider.setRange (0.5, 2.0);
    speedSlider.setValue (1.0);

    resamplingSource = std::make_unique<juce::ResamplingAudioSource>(&transportSource, false);

    speedSlider.onValueChange = [this] {
        if (resamplingSource != nullptr)
            resamplingSource->setResamplingRatio (speedSlider.getValue());
    };

    addAndMakeVisible(speedLabel);
    
    addAndMakeVisible(recordingThumbnail);

    setSize (500, 500);

    formatManager.registerBasicFormats();
    transportSource.addChangeListener (this);

    setAudioChannels (2, 2);
    
    recorder.reset(new AudioRecorder(recordingThumbnail.getAudioThumbnail()));
    deviceManager.addAudioCallback (recorder.get());
}

MainComponent::~MainComponent()
{
    transportSource.removeChangeListener (this);
    deviceManager.removeAudioCallback(recorder.get());
    shutdownAudio();
    transportSource.stop();
    transportSource.setSource(nullptr);
}

void MainComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    resamplingSource->prepareToPlay (samplesPerBlockExpected, sampleRate);
}

void MainComponent::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill)
{
    resamplingSource->getNextAudioBlock (bufferToFill);
}

void MainComponent::releaseResources()
{
    resamplingSource->releaseResources();
}

void MainComponent::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void MainComponent::resized()
{
    auto area = getLocalBounds();
    recordingThumbnail.setBounds (area.removeFromTop (80).reduced (8));
    recordButton.setBounds (area.removeFromTop (36).removeFromLeft (140).reduced (8));
    playButton.setBounds (area.removeFromTop (36).removeFromLeft (140).reduced (8));
    
    auto sliderArea = area.removeFromTop(36);
    speedLabel.setBounds(sliderArea.removeFromLeft(50).reduced(8));
    speedSlider.setBounds(sliderArea.reduced(8));
}

void MainComponent::changeListenerCallback (juce::ChangeBroadcaster* source)
{
    if (source == &transportSource)
    {
        if (transportSource.isPlaying())
            transportStateChanged (Playing);
        else
            transportStateChanged (Stopped);
    }
}

void MainComponent::startRecording()
{
    auto parentDir = juce::File::getSpecialLocation (juce::File::userDocumentsDirectory);
    currentRecordingFile = parentDir.getNonexistentChildFile ("AudioRecording", ".wav");
    recorder->startRecording (currentRecordingFile);
    recordButton.setButtonText ("Stop");
    playButton.setEnabled (false);
}

void MainComponent::stopRecording()
{
    recorder->stop();
    recordButton.setButtonText ("Record");
    playButton.setEnabled (true);
}

void MainComponent::startPlaying()
{
    transportSource.stop();
    transportSource.setSource (nullptr);

    if (currentRecordingFile.existsAsFile())
    {
        if (auto* reader = formatManager.createReaderFor (currentRecordingFile))
        {
            transportSource.setSource (new juce::AudioFormatReaderSource (reader, true), 0, nullptr, reader->sampleRate);
            transportStateChanged (Starting);
        }
    }
}

void MainComponent::stopPlaying()
{
    transportStateChanged(Stopping);
}

void MainComponent::transportStateChanged(TransportState newState)
{
    if (newState != state)
    {
        state = newState;

        switch (state)
        {
            case Stopped:
                playButton.setButtonText ("Play");
                transportSource.setPosition (0.0);
                break;

            case Starting:
                playButton.setEnabled (false);
                transportSource.start();
                break;

            case Playing:
                playButton.setButtonText ("Stop");
                playButton.setEnabled (true);
                break;

            case Stopping:
                playButton.setEnabled (false);
                transportSource.stop();
                break;
        }
    }
}
