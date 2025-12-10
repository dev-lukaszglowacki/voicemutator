#include "AudioRecorder.h"

AudioRecorder::AudioRecorder (juce::AudioThumbnail& thumbnailToUpdate)
    : thumbnail (thumbnailToUpdate)
{
    backgroundThread.startThread();
}

AudioRecorder::~AudioRecorder()
{
    stop();
}

void AudioRecorder::startRecording (const juce::File& file)
{
    stop();

    if (sampleRate > 0)
    {
        file.deleteFile();

        if (auto fileStream = file.createOutputStream())
        {
            juce::WavAudioFormat wavFormat;

            if (auto* writer = wavFormat.createWriterFor (fileStream.get(), sampleRate, 1, 16, {}, 0))
            {
                fileStream.release();

                threadedWriter.reset (new juce::AudioFormatWriter::ThreadedWriter (writer, backgroundThread, 32768));

                thumbnail.reset (writer->getNumChannels(), writer->getSampleRate());
                nextSampleNum = 0;

                const juce::ScopedLock sl (writerLock);
                activeWriter = threadedWriter.get();
            }
        }
    }
}

void AudioRecorder::stop()
{
    {
        const juce::ScopedLock sl (writerLock);
        activeWriter = nullptr;
    }

    threadedWriter.reset();
}

bool AudioRecorder::isRecording() const
{
    return activeWriter.load() != nullptr;
}

void AudioRecorder::audioDeviceAboutToStart (juce::AudioIODevice* device)
{
    sampleRate = device->getCurrentSampleRate();
}

void AudioRecorder::audioDeviceStopped()
{
    sampleRate = 0;
}

void AudioRecorder::audioDeviceIOCallbackWithContext (const float* const* inputChannelData, int numInputChannels,
                                       float* const* outputChannelData, int numOutputChannels,
                                       int numSamples, const juce::AudioIODeviceCallbackContext& context)
{
    juce::ignoreUnused (context);

    const juce::ScopedLock sl (writerLock);

    if (activeWriter.load() != nullptr && numInputChannels >= thumbnail.getNumChannels())
    {
        activeWriter.load()->write (inputChannelData, numSamples);

        juce::AudioBuffer<float> buffer (const_cast<float**> (inputChannelData), thumbnail.getNumChannels(), numSamples);
        thumbnail.addBlock (nextSampleNum, buffer, 0, numSamples);
        nextSampleNum += numSamples;
    }

    for (int i = 0; i < numOutputChannels; ++i)
        if (outputChannelData[i] != nullptr)
            juce::FloatVectorOperations::clear (outputChannelData[i], numSamples);
}
