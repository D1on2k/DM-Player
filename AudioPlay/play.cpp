/*
MIT License

Copyright (c) 2026 Dion2k

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#define MINIAUDIO_IMPLEMENTATION

#include "Includes/miniaudio.c"
#include "play.h"

using namespace std;

struct playmusic
{
    ma_decoder decoder;
    ma_device device;

    string csongpath;

    bool isitplaying;
    bool songfinished;
};

playmusic* g_playit = nullptr; // future me it will be used a lot 

void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
{
    playmusic* p = (playmusic*) pDevice->pUserData;

    if (p == NULL || !p->isitplaying)
    {
        memset(pOutput, 0, frameCount * ma_get_bytes_per_frame(ma_format_f32, 2)); // Do it that way so we fill in the silence 

        return;
    }

    ma_uint64 readframes = 0;
    ma_decoder_read_pcm_frames(&p->decoder, pOutput, frameCount, &readframes);

    if (readframes < frameCount)
    {
        // FIll the rest of the frames with silence so no tss plays in 
        ma_uint32 remain = frameCount - (ma_uint32)readframes;
        size_t perframebytes = ma_get_bytes_per_frame(p->decoder.outputFormat, p->decoder.outputChannels);

        memset((char*) pOutput + readframes * perframebytes, 0, remain * perframebytes);

        p->songfinished = true;
        p->isitplaying = false;
    }

    (void)pInput;
}

bool playerinitilize()
{
    if (g_playit != nullptr)
    {
        return true;
    }

    // yes we will do manuall memory management the good old way no new smart pointers 
    g_playit = new playmusic();
    g_playit->isitplaying = false;
    g_playit->songfinished = false;

    return true;
}

bool playerplay(const string& filepath)
{
    if (g_playit == nullptr) return false;

    if (g_playit->isitplaying)
    {
        playerstop();
    }

    if (ma_decoder_init_file(filepath.c_str(), NULL, &g_playit->decoder) != MA_SUCCESS)
    {
        printf("Couldnt load file: %s \n", filepath.c_str());
        return false;
    }

    ma_device_config deviceConfig = ma_device_config_init(ma_device_type_playback);
    deviceConfig.playback.format = g_playit->decoder.outputFormat;
    deviceConfig.playback.channels = g_playit->decoder.outputChannels;
    deviceConfig.sampleRate = g_playit->decoder.outputSampleRate;
    deviceConfig.dataCallback = data_callback;
    deviceConfig.pUserData = g_playit;

    if (ma_device_init(NULL, &deviceConfig, &g_playit->device) != MA_SUCCESS)
    {
        printf("Failed to open playback device.\n");
        ma_decoder_uninit(&g_playit->decoder);
        return false;
    }

    if (ma_device_start(&g_playit->device) != MA_SUCCESS)
    {
        printf("Failed to start playback.\n");
        ma_device_uninit(&g_playit->device);
        ma_decoder_uninit(&g_playit->decoder);
        return false;
    }

    g_playit->csongpath = filepath;
    g_playit->isitplaying = true;
    g_playit->songfinished = false;

    printf("Successfully started: %s\n", filepath.c_str());
    return true;

    // here i shouldve add a thing that says time but i will add it on the ui later
}
    

bool playerpause()
{
    if (g_playit == nullptr)
    {
        return false;
    }

    g_playit->isitplaying = false;
    
    return true;
}

// here is the good part optimize it so if nothing is loaded it will stop 
bool playerstop()
{
    if (g_playit == nullptr)
        return false;

    ma_device_stop(&g_playit->device);
    ma_device_uninit(&g_playit->device);
    ma_decoder_uninit(&g_playit->decoder);

    g_playit->isitplaying = false;

    return true;
}
bool playerifsongjustfinished()
{
    if (g_playit == nullptr)
    {
        return false;
    }

    if (g_playit->songfinished)
    {
        g_playit->songfinished = false;

        return true;
    }

    return false;
}

std::string playergettime()
{
    if (g_playit == nullptr)
    {
        return "";
    }

    return g_playit->csongpath;
}

float volume = 0.5f; // I tried with 1.0 it was an ear rape defenetly not recomnded

bool setvolume(float okbradar)
{
    if (g_playit == nullptr) 
    {
        return false;
    }

    if (volume < 0.0f) volume = 0.0f;
    if (volume > 1.0f) volume = 1.0f;

    volume = okbradar;

    ma_device_set_master_volume(&g_playit->device, volume);
    
    return true;
}

float getvolume()
{
    if (g_playit == nullptr) 
    {
        return 1.0f;
    }
    
    ma_device_get_master_volume(&g_playit->device, &volume);
    
    return volume;
}

void asdasdcleanup()
{
    if (g_playit != nullptr)
    {
        if (g_playit->isitplaying)
        {
            playerstop();
        }

        delete g_playit; // alsmost forgot this damn thing
        g_playit = nullptr;
    }
}
