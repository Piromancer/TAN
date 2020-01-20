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
#pragma once

#include "IAudio3D.h"

class Audio3DAMF:
    public IAudio3D
{
protected:
    AMFContextPtr               mContext12;
    AMFComputePtr               mCompute1;
    AMFComputePtr               mCompute2;

    AMFContextPtr               mContext3;
    AMFComputePtr               mCompute3;

    bool                        mUseAMFBuffers = false;
    AMFBufferPtr                mAMFResponses[MAX_SOURCES * 2];

    AMFBufferPtr                mOutputAMFBufers[MAX_SOURCES * 2] = {nullptr};
	AMFBufferPtr                mOutputMainAMFBuffer = nullptr;
	AMFBufferPtr                mOutputMixAMFBuffer[2] = {nullptr};
	AMFBufferPtr                mOutputShortBuffer = nullptr;

    int Process(int16_t *pOut, int16_t *pChan[MAX_SOURCES], uint32_t sampleCountBytes);
    int ProcessProc();
    int UpdateProc();

    static unsigned processThreadProc(void *ptr);
    static unsigned updateThreadProc(void *ptr);

public:
    Audio3DAMF();
    Audio3DAMF(Audio3DAMF const &) = delete;
    virtual ~Audio3DAMF();

    bool Init
    (
        const std::string &     dllPath,
        const RoomDefinition &  roomDef,
        const std::vector<std::string> &
                                fileNames2Open,

        // Source 1 can optionally be captured audio from default microphone:
        bool                    useMicSource,
        const std::vector<bool> &
                                trackHeadPos,

        int                     fftLen,
        int                     bufSize,

        bool                    useCLConvolution,
        bool                    useGPUConvolution,
        int                     deviceIndexConvolution,

#ifdef RTQ_ENABLED
		bool                    useHPr_Conv,
        bool                    useRTQ_Conv,
        int                     cuRes_Conv,
#endif // RTQ_ENABLED

        bool                    useCLRoom,
        bool                    useGPURoom,
        int                     deviceIndexRoom,

#ifdef RTQ_ENABLED
		bool                    useHPr_IRGen,
        bool                    useRTQ_IRGen,
        int                     cuRes_IRGen,
#endif

        amf::TAN_CONVOLUTION_METHOD
                                convMethod,

        const std::string &     playerType,

        RoomUpdateMode          roomUpdateMode = RoomUpdateMode::Blocking
        ) override;

	// finalize, deallocate resources, close files, etc.
	void Close() override;

	// start audio engine:
    bool Run() override;

	// Stop audio engine:
    void Stop() override;

public:
    //todo: move to Init with help of struct
    // Set transform to translate game coordinates to room audio coordinates:
	virtual int setWorldToRoomCoordTransform(
        float translationX,
        float translationY,
        float translationZ,
        float rotationY,
        float headingOffset,
        bool headingCCW
        );

	// get's the current playback position in a stream:
    //int64_t getCurrentPosition(int stream);

	// update the head (listener) position:
    virtual int updateHeadPosition(float x, float y, float z, float yaw, float pitch, float roll);

	//update a source position:
    virtual int updateSourcePosition(int srcNumber, float x, float y, float z);

	//update a room's dimension:
	virtual int updateRoomDimension(float _width, float _height, float _length);

	//update a room's damping factor
	virtual int updateRoomDamping(float _left, float _right, float _top, float _buttom, float _front, float _back);

    // export impulse response for source  + current listener and room:
    virtual int exportImpulseResponse(int srcNumber, char * fname);
};