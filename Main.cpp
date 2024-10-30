#include "juce_audio_devices/juce_audio_devices.h"
#include "juce_events/juce_events.h"
#include <juce_core/juce_core.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include "choc_MessageLoop.h"
#include "choc_DesktopWindow.h"
#include "choc_WebView.h"

/*
I don't like the Juce AudioSources stuff, so I did this low level audio callback class
instead, which will be used used by the AudioDeviceManager.

We inherit from timer only to confirm that async Juce stuff like Timer do work
under the Choc event loop, it is not generally required
*/
class MyAudioCallback : public juce::AudioIODeviceCallback, public juce::Timer
{
  public:
    MyAudioCallback() { startTimer(1000); }
    void timerCallback() override { DBG("Timer callback"); }
    void
    audioDeviceIOCallbackWithContext(const float *const */*inputChannelData*/, int /*numInputChannels*/,
                                     float *const *outputChannelData, int numOutputChannels,
                                     int numSamples,
                                     const juce::AudioIODeviceCallbackContext &/*context*/) override
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
    void audioDeviceAboutToStart(juce::AudioIODevice *device) override {}
    void audioDeviceStopped() override {}
    juce::Random m_rng;
};

int main(int /*argc*/, char */*argv*/[])
{
    // This doesn't really do much else than ensure that the Juce MessageManager instance will be around,
    // doesn't imply Juce GUI will be used/needs to be used
    juce::ScopedJuceInitialiser_GUI guiInit;
    
    // Might be safer to heap allocate the manager and callback, but stack allocated works
    // for this simple example
    juce::AudioDeviceManager mana;
    mana.initialiseWithDefaultDevices(0, 2);
    MyAudioCallback audioCallback;
    mana.addAudioCallback(&audioCallback);
    
    // Create a Choc based desktop window and event loop. 
    // Audio keeps playing until the window is closed.
    choc::messageloop::initialise();
    choc::ui::DesktopWindow dwin({50, 50, 400, 200});
    dwin.setWindowTitle("Choc window");
    dwin.windowClosed = []() { choc::messageloop::stop(); };
    choc::messageloop::run();
    return 0;
}
