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
    speedLabel.setColour (juce::Label::textColourId, juce::Colours::black);
    
    addAndMakeVisible (delayFeedbackSlider);
    delayFeedbackSlider.setRange (0.0, 0.99);
    delayFeedbackSlider.setValue (0.0);
    addAndMakeVisible(delayFeedbackLabel);
    delayFeedbackLabel.setColour (juce::Label::textColourId, juce::Colours::black);

    addAndMakeVisible (delayWetDrySlider);
    delayWetDrySlider.setRange (0.0, 1.0);
    delayWetDrySlider.setValue (0.0);
    addAndMakeVisible(delayWetDryLabel);
    delayWetDryLabel.setColour (juce::Label::textColourId, juce::Colours::black);

    addAndMakeVisible (reverbRoomSizeSlider);
    reverbRoomSizeSlider.setRange (0.0, 1.0);
    reverbRoomSizeSlider.setValue (0.5);
    addAndMakeVisible(reverbRoomSizeLabel);
    reverbRoomSizeLabel.setColour (juce::Label::textColourId, juce::Colours::black);

    addAndMakeVisible (reverbWetDrySlider);
    reverbWetDrySlider.setRange (0.0, 1.0);
    reverbWetDrySlider.setValue (0.0);
    addAndMakeVisible(reverbWetDryLabel);
    reverbWetDryLabel.setColour (juce::Label::textColourId, juce::Colours::black);
    
    addAndMakeVisible(reverbDampingSlider);
    reverbDampingSlider.setRange(0.0, 1.0);
    reverbDampingSlider.setValue(0.5);
    addAndMakeVisible(reverbDampingLabel);
    reverbDampingLabel.setColour (juce::Label::textColourId, juce::Colours::black);

    addAndMakeVisible(reverbWidthSlider);
    reverbWidthSlider.setRange(0.0, 1.0);
    reverbWidthSlider.setValue(1.0);
    addAndMakeVisible(reverbWidthLabel);
    reverbWidthLabel.setColour (juce::Label::textColourId, juce::Colours::black);

    addAndMakeVisible(flangerRateSlider);
    flangerRateSlider.setRange(0.0, 10.0);
    flangerRateSlider.setValue(1.0);
    addAndMakeVisible(flangerRateLabel);
    flangerRateLabel.setColour (juce::Label::textColourId, juce::Colours::black);

    addAndMakeVisible(flangerDepthSlider);
    flangerDepthSlider.setRange(0.0, 1.0);
    flangerDepthSlider.setValue(0.5);
    addAndMakeVisible(flangerDepthLabel);
    flangerDepthLabel.setColour (juce::Label::textColourId, juce::Colours::black);

    addAndMakeVisible(flangerFeedbackSlider);
    flangerFeedbackSlider.setRange(0.0, 0.99);
    flangerFeedbackSlider.setValue(0.0);
    addAndMakeVisible(flangerFeedbackLabel);
    flangerFeedbackLabel.setColour (juce::Label::textColourId, juce::Colours::black);

    addAndMakeVisible(flangerMixSlider);
    flangerMixSlider.setRange(0.0, 1.0);
    flangerMixSlider.setValue(0.0);
    addAndMakeVisible(flangerMixLabel);
    flangerMixLabel.setColour (juce::Label::textColourId, juce::Colours::black);

    addAndMakeVisible (distortionDriveSlider);
    distortionDriveSlider.setRange (1.0, 10.0);
    distortionDriveSlider.setValue (1.0);
    addAndMakeVisible(distortionDriveLabel);
    distortionDriveLabel.setColour (juce::Label::textColourId, juce::Colours::black);

    addAndMakeVisible (distortionMixSlider);
    distortionMixSlider.setRange (0.0, 1.0);
    distortionMixSlider.setValue (0.0);
    addAndMakeVisible(distortionMixLabel);
    distortionMixLabel.setColour (juce::Label::textColourId, juce::Colours::black);

    addAndMakeVisible(recordingThumbnail);

    setSize (500, 800);

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
    currentSampleRate = sampleRate;
    resamplingSource->prepareToPlay (samplesPerBlockExpected, sampleRate);
    
    delayLine = std::make_unique<juce::dsp::DelayLine<float>>(sampleRate);
    delayLine->prepare({ sampleRate, (juce::uint32) samplesPerBlockExpected, 2 });
    delayLine->setMaximumDelayInSamples(sampleRate);

    reverb = std::make_unique<juce::dsp::Reverb>();
    reverb->prepare({ sampleRate, (juce::uint32) samplesPerBlockExpected, 2 });

    flanger = std::make_unique<juce::dsp::Chorus<float>>();
    flanger->prepare({ sampleRate, (juce::uint32) samplesPerBlockExpected, 2 });

    oversampling = std::make_unique<juce::dsp::Oversampling<float>>(2, 4, juce::dsp::Oversampling<float>::FilterType::filterHalfBandFIREquiripple);
    oversampling->initProcessing(samplesPerBlockExpected);
}

void MainComponent::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill)
{
    resamplingSource->getNextAudioBlock (bufferToFill);

    if (delayLine != nullptr)
    {
        auto* leftChannel = bufferToFill.buffer->getWritePointer(0);
        auto* rightChannel = bufferToFill.buffer->getWritePointer(1);

        auto feedback = delayFeedbackSlider.getValue();
        auto wet = delayWetDrySlider.getValue();

        if (wet > 0.0)
        {
            auto dry = 1.0 - wet;

            for (int i = 0; i < bufferToFill.numSamples; ++i)
            {
                delayLine->setDelay(0.5 * currentSampleRate);

                auto delayedLeft = delayLine->popSample(0);
                auto delayedRight = delayLine->popSample(1);

                auto mixedLeft = (leftChannel[i] * dry) + (delayedLeft * wet);
                auto mixedRight = (rightChannel[i] * dry) + (delayedRight * wet);

                delayLine->pushSample(0, leftChannel[i] + delayedLeft * feedback);
                delayLine->pushSample(1, rightChannel[i] + delayedRight * feedback);

                leftChannel[i] = mixedLeft;
                rightChannel[i] = mixedRight;
            }
        }
    }

    if (reverb != nullptr)
    {
        juce::dsp::Reverb::Parameters params;
        params.roomSize = reverbRoomSizeSlider.getValue();
        params.wetLevel = reverbWetDrySlider.getValue();
        params.dryLevel = 1.0f - params.wetLevel;
        params.damping = reverbDampingSlider.getValue();
        params.width = reverbWidthSlider.getValue();
        reverb->setParameters(params);

        juce::dsp::AudioBlock<float> block (*(bufferToFill.buffer));
        juce::dsp::ProcessContextReplacing<float> context (block);
        reverb->process(context);
    }
    
    if (flanger != nullptr)
    {
        flanger->setRate(flangerRateSlider.getValue());
        flanger->setDepth(flangerDepthSlider.getValue());
        flanger->setFeedback(flangerFeedbackSlider.getValue());
        flanger->setMix(flangerMixSlider.getValue());

        juce::dsp::AudioBlock<float> block (*(bufferToFill.buffer));
        juce::dsp::ProcessContextReplacing<float> context (block);
        flanger->process(context);
    }

    if (oversampling != nullptr && distortionMixSlider.getValue() > 0.0)
    {
        juce::dsp::AudioBlock<float> block (*(bufferToFill.buffer));
        juce::dsp::AudioBlock<float> oversampledBlock = oversampling->processSamplesUp(block);

        auto* leftChannel = oversampledBlock.getChannelPointer(0);
        auto* rightChannel = oversampledBlock.getChannelPointer(1);
        auto drive = distortionDriveSlider.getValue();

        for (int i = 0; i < oversampledBlock.getNumSamples(); ++i)
        {
            leftChannel[i] = std::tanh(leftChannel[i] * drive);
            rightChannel[i] = std::tanh(rightChannel[i] * drive);
        }

        oversampling->processSamplesDown(block);
        
        auto mix = distortionMixSlider.getValue();
        auto* originalLeft = bufferToFill.buffer->getReadPointer(0);
        auto* originalRight = bufferToFill.buffer->getReadPointer(1);
        auto* distortedLeft = block.getChannelPointer(0);
        auto* distortedRight = block.getChannelPointer(1);

        for (int i = 0; i < bufferToFill.numSamples; ++i)
        {
            distortedLeft[i] = originalLeft[i] * (1.0f - mix) + distortedLeft[i] * mix;
            distortedRight[i] = originalRight[i] * (1.0f - mix) + distortedRight[i] * mix;
        }
    }
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
    speedLabel.setBounds(sliderArea.removeFromLeft(120).reduced(8));
    speedSlider.setBounds(sliderArea.reduced(8));

    auto feedbackSliderArea = area.removeFromTop(36);
    delayFeedbackLabel.setBounds(feedbackSliderArea.removeFromLeft(120).reduced(8));
    delayFeedbackSlider.setBounds(feedbackSliderArea.reduced(8));

    auto wetDrySliderArea = area.removeFromTop(36);
    delayWetDryLabel.setBounds(wetDrySliderArea.removeFromLeft(120).reduced(8));
    delayWetDrySlider.setBounds(wetDrySliderArea.reduced(8));

    auto roomSizeSliderArea = area.removeFromTop(36);
    reverbRoomSizeLabel.setBounds(roomSizeSliderArea.removeFromLeft(120).reduced(8));
    reverbRoomSizeSlider.setBounds(roomSizeSliderArea.reduced(8));

    auto reverbWetDrySliderArea = area.removeFromTop(36);
    reverbWetDryLabel.setBounds(reverbWetDrySliderArea.removeFromLeft(120).reduced(8));
    reverbWetDrySlider.setBounds(reverbWetDrySliderArea.reduced(8));

    auto reverbDampingArea = area.removeFromTop(36);
    reverbDampingLabel.setBounds(reverbDampingArea.removeFromLeft(120).reduced(8));
    reverbDampingSlider.setBounds(reverbDampingArea.reduced(8));

    auto reverbWidthArea = area.removeFromTop(36);
    reverbWidthLabel.setBounds(reverbWidthArea.removeFromLeft(120).reduced(8));
    reverbWidthSlider.setBounds(reverbWidthArea.reduced(8));

    auto flangerRateArea = area.removeFromTop(36);
    flangerRateLabel.setBounds(flangerRateArea.removeFromLeft(120).reduced(8));
    flangerRateSlider.setBounds(flangerRateArea.reduced(8));

    auto flangerDepthArea = area.removeFromTop(36);
    flangerDepthLabel.setBounds(flangerDepthArea.removeFromLeft(120).reduced(8));
    flangerDepthSlider.setBounds(flangerDepthArea.reduced(8));

    auto flangerFeedbackArea = area.removeFromTop(36);
    flangerFeedbackLabel.setBounds(flangerFeedbackArea.removeFromLeft(120).reduced(8));
    flangerFeedbackSlider.setBounds(flangerFeedbackArea.reduced(8));

    auto flangerMixArea = area.removeFromTop(36);
    flangerMixLabel.setBounds(flangerMixArea.removeFromLeft(120).reduced(8));
    flangerMixSlider.setBounds(flangerMixArea.reduced(8));

    auto distortionDriveArea = area.removeFromTop(36);
    distortionDriveLabel.setBounds(distortionDriveArea.removeFromLeft(120).reduced(8));
    distortionDriveSlider.setBounds(distortionDriveArea.reduced(8));

    auto distortionMixArea = area.removeFromTop(36);
    distortionMixLabel.setBounds(distortionMixArea.removeFromLeft(120).reduced(8));
    distortionMixSlider.setBounds(distortionMixArea.reduced(8));
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
    if (delayLine != nullptr)
        delayLine->reset();
    if (reverb != nullptr)
        reverb->reset();
    if (flanger != nullptr)
        flanger->reset();
    if (oversampling != nullptr)
        oversampling->reset();
        
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
