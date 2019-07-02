//
// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#ifndef _WASAPI_H_
#define _WASAPI_H_

#include "IWavPlayer.h"

#include <Windows.h>
//#include <VersionHelpers.h>
#include <iostream>
#include <mmdeviceapi.h>
#include <Audioclient.h>
#include <mmsystem.h>
#include <Audioclient.h>
#include <endpointvolume.h>

class WASAPIPlayer:
  public IWavPlayer
{
protected:
    uint16_t mChannelsCount;
    uint16_t mBitsPerSample;
    uint32_t mSamplesPerSecond;

    IMMDevice *devRender;
    IMMDevice *devCapture;
    IMMDeviceEnumerator *devEnum;
    IAudioClient *audioClient;
    IAudioRenderClient *renderClient;
    IAudioClient *audioCapClient;
    IAudioCaptureClient *captureClient;
    UINT frameSize;
    UINT32 bufferSize;
    bool mStartedRender;
    bool mStartedCapture;
    bool mInitializedRender;
    bool mInitializedCapture;

public:
    WASAPIPlayer();
    virtual ~WASAPIPlayer();

    std::string GetPlayerName() const override { return "WASAPI"; }

    virtual PlayerError Init(
        uint16_t    channelsCount,
        uint16_t    bitsPerSample,
        uint32_t    samplesPerSecond,
        bool        play,
        bool        record
    ) override;
    virtual void Close() override;

    virtual uint32_t Play(uint8_t * buffer, uint32_t size, bool mute) override;
    virtual uint32_t Record(uint8_t * buffer, uint32_t size) override;
};

#endif