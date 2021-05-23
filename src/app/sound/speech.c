#include "speech.h"

#include "app/core/dir.h"
#include "app/game/settings.h"
#include "app/sound/channel.h"
#include "app/sound/device.h"

void sound_speech_set_volume(int percentage)
{
    sound_device_set_channel_volume(SOUND_CHANNEL_SPEECH, percentage);
}

void sound_speech_play_file(const char *filename)
{
    if (!setting_sound(SOUND_SPEECH)->enabled) {
        return;
    }
    sound_device_stop_channel(SOUND_CHANNEL_SPEECH);

    const char *cased_filename = dir_get_file(filename, MAY_BE_LOCALIZED);
    if (cased_filename) {
        sound_device_play_file_on_channel(cased_filename, SOUND_CHANNEL_SPEECH, setting_sound(SOUND_SPEECH)->volume);
    }
}

void sound_speech_stop(void)
{
    sound_device_stop_channel(SOUND_CHANNEL_SPEECH);
}
