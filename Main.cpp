#include "juce_audio_basics/juce_audio_basics.h"
#include "juce_audio_devices/juce_audio_devices.h"
#include "juce_events/juce_events.h"
#include <juce_core/juce_core.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include "choc_MessageLoop.h"
#include "choc_DesktopWindow.h"

/*
I don't like the Juce AudioSources stuff, so I did this low level audio callback class
instead, which will be used used by the AudioDeviceManager.

This also inherits from Timer and AsyncUpdater only to confirm that the async Juce stuff
does work under the Choc message loop, it is not generally required.
*/

class MyAudioCallback : public juce::AudioIODeviceCallback,
                        public juce::Timer,
                        public juce::AsyncUpdater
{
  public:
    MyAudioCallback() { startTimer(1000); }
    void timerCallback() override
    {
        jassert(juce::MessageManager::getInstance()->isThisTheMessageThread());
        DBG("Timer callback");
    }
    void handleAsyncUpdate() override
    {
        jassert(juce::MessageManager::getInstance()->isThisTheMessageThread());
        DBG("AsyncUpdater callback");
    }
    // Equivalent to AudioSource::getNextAudioBlock/AudioProcessor::processBlock
    void audioDeviceIOCallbackWithContext(
        const float *const * /*inputChannelData*/, int /*numInputChannels*/,
        float *const *outputChannelData, int numOutputChannels, int numSamples,
        const juce::AudioIODeviceCallbackContext & /*context*/) override
    {
        // Cache the value locally so we don't have to access the atomic inside the loop
        float gain = m_gain.load();
        // Generate some white noise into the output channels.
        for (int i = 0; i < numSamples; ++i)
        {
            float outSample = gain * juce::jmap(m_rng.nextFloat(), 0.0f, 1.0f, -1.0f, 1.0f);
            for (int j = 0; j < numOutputChannels; ++j)
            {
                outputChannelData[j][i] = outSample;
            }
        }
        m_samplePosition += numSamples;
        if (m_samplePosition >= 88200)
        {
            m_samplePosition = 0;
            // Note : it's controversial whether AsyncUpdater should be used in the audio thread or
            // not. You may not want to do this in production code.
            triggerAsyncUpdate();
        }
    }
    // Equivalent to AudioSource/AudioProcessor::prepareToPlay
    void audioDeviceAboutToStart(juce::AudioIODevice * /*device*/) override {}

    // Equivalent to AudioSource/AudioProcessor::releaseResources
    void audioDeviceStopped() override {}

    void setVolume(float decibels)
    {
        decibels = juce::jlimit(-100.0f, 0.0f, decibels);
        m_gain.store(juce::Decibels::decibelsToGain(decibels));
    }

  private:
    juce::Random m_rng;
    std::atomic<float> m_gain{0.0f};
    int64_t m_samplePosition = 0;
};

int main(int /*argc*/, char * /*argv*/[])
{
    // This doesn't really do much else than ensure that the Juce MessageManager instance will be
    // around, doesn't imply Juce GUI will be used/needs to be used
    juce::ScopedJuceInitialiser_GUI guiInit;

    auto mana = std::make_unique<juce::AudioDeviceManager>();
    mana->initialiseWithDefaultDevices(0, 2);
    auto audioCallback = std::make_unique<MyAudioCallback>();
    audioCallback->setVolume(-36.0f);
    mana->addAudioCallback(audioCallback.get());

    // Create a Choc based desktop window and start the message loop.
    // Audio keeps playing until the window is closed.
    choc::messageloop::initialise();
    auto dwin = std::make_unique<choc::ui::DesktopWindow>(choc::ui::Bounds{50, 50, 400, 200});
    dwin->setWindowTitle("Choc window");
    dwin->windowClosed = []() { choc::messageloop::stop(); };
    choc::messageloop::run(); // blocking here until the window is closed
    mana->removeAudioCallback(audioCallback.get());
    return 0;
}
