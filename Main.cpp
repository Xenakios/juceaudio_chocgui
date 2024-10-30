#include "juce_audio_devices/juce_audio_devices.h"
#include "juce_events/juce_events.h"
#include <juce_core/juce_core.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include "choc_MessageLoop.h"
#include "choc_DesktopWindow.h"

/*
I don't like the Juce AudioSources stuff, so I did this low level audio callback class
instead, which will be used used by the AudioDeviceManager.

This also inherits from timer only to confirm that async Juce stuff like Timer does work
under the Choc event loop, it is not generally required.
*/

class MyAudioCallback : public juce::AudioIODeviceCallback, public juce::Timer
{
  public:
    MyAudioCallback() { startTimer(1000); }
    void timerCallback() override { DBG("Timer callback"); }
    void audioDeviceIOCallbackWithContext(
        const float *const * /*inputChannelData*/, int /*numInputChannels*/,
        float *const *outputChannelData, int numOutputChannels, int numSamples,
        const juce::AudioIODeviceCallbackContext & /*context*/) override
    {
        // generate some low level noise into the output channels
        for (int i = 0; i < numSamples; ++i)
        {
            float outSample = juce::jmap(m_rng.nextFloat(), 0.0f, 1.0f, -0.05f, 0.05f);
            for (int j = 0; j < numOutputChannels; ++j)
            {
                outputChannelData[j][i] = outSample;
            }
        }
    }
    // equivalent to AudioSource/AudioProcessor::prepareToPlay
    void audioDeviceAboutToStart(juce::AudioIODevice * /*device*/) override {}

    void audioDeviceStopped() override {}

  private:
    juce::Random m_rng;
};

int main(int /*argc*/, char * /*argv*/[])
{
    // This doesn't really do much else than ensure that the Juce MessageManager instance will be
    // around, doesn't imply Juce GUI will be used/needs to be used
    juce::ScopedJuceInitialiser_GUI guiInit;

    auto mana = std::make_unique<juce::AudioDeviceManager>();
    mana->initialiseWithDefaultDevices(0, 2);
    auto audioCallback = std::make_unique<MyAudioCallback>();
    mana->addAudioCallback(audioCallback.get());

    // Create a Choc based desktop window and start the event loop.
    // Audio keeps playing until the window is closed.
    choc::messageloop::initialise();
    auto dwin = std::make_unique<choc::ui::DesktopWindow>(choc::ui::Bounds{50, 50, 400, 200});
    dwin->setWindowTitle("Choc window");
    dwin->windowClosed = []() { choc::messageloop::stop(); };
    choc::messageloop::run(); // blocking here until the window is closed
    mana->removeAudioCallback(audioCallback.get());
    return 0;
}
