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

#include <stdio.h>
#include <memory.h>
#include "wav.h"

#ifdef __unix
#define fopen_s(pFile,filename,mode) ((*(pFile))=fopen((filename),  (mode)))==NULL
#endif

void SetupWaveHeader
(
	RiffWave *fhd,
	int32_t sampleRate,
	int32_t bitsPerSample,
	int channels,
	long samplesCount)
{
	long dataLength;
	dataLength = samplesCount*(bitsPerSample / 8)*channels;
	memset(fhd, 0, sizeof(RiffWave));

	fhd->riff.name[0] = 'R';
	fhd->riff.name[1] = 'I';
	fhd->riff.name[2] = 'F';
	fhd->riff.name[3] = 'F';

	fhd->wave.name[0] = 'W';
	fhd->wave.name[1] = 'A';
	fhd->wave.name[2] = 'V';
	fhd->wave.name[3] = 'E';

	fhd->wave.data.name[0] = 'd';
	fhd->wave.data.name[1] = 'a';
	fhd->wave.data.name[2] = 't';
	fhd->wave.data.name[3] = 'a';

	fhd->wave.fmt.name[0] = 'f';
	fhd->wave.fmt.name[1] = 'm';
	fhd->wave.fmt.name[2] = 't';
	fhd->wave.fmt.name[3] = ' ';

	/* set the data size				*/
	fhd->wave.data.length = dataLength;

	/* set the RIFF length				*/
	fhd->riff.length = dataLength + sizeof(WaveHeader);

	/* set the length of the FORMAT block	*/
	fhd->wave.fmt.length = sizeof(WaveInfo);
	fhd->wave.fmt.info.formatTag = 1;

	/* set up the sample rate, etc...	*/
	fhd->wave.fmt.info.nChannels = (uint16_t)channels;
	fhd->wave.fmt.info.nSamplesPerSec = sampleRate;

	//fhd->wave.fmt.info.nAvgBytesPerSec = (sampleRate << (channels - 1)) <<
	//	((bitsPerSample == 8) ? 0 : 1);
	//fhd->wave.fmt.info.nBlockAlign = (1 + ((bitsPerSample == 8) ? 0 : 1))
	//	<< (channels - 1);

    fhd->wave.fmt.info.nAvgBytesPerSec = (sampleRate*channels*bitsPerSample) / 8;
    fhd->wave.fmt.info.nBlockAlign = (channels*bitsPerSample) / 8;

    if (bitsPerSample == 32) {
        fhd->wave.fmt.info.formatTag = 3;
    }

	fhd->wave.fmt.info.nBitsPerSample = (uint16_t)bitsPerSample;
}

bool 			ReadWaveFile
(
	const char *fileName,
	uint32_t &	samplesPerSec,
	uint16_t &	bitsPerSample,
	uint16_t &	channelsCount,
	uint32_t &	samplesCount,
	uint8_t **	pSamples,
	float ***	pfSamples
)
{
	//reset output
	samplesPerSec = 0;
	bitsPerSample = 0;
	channelsCount = 0;
	samplesCount = 0;
	*pSamples = nullptr;

	FILE *fpIn = nullptr;
	if ((fopen_s(&fpIn,fileName, "rb")) != 0)
	{
		printf("ReadWaveFile: Can't open %s\n", fileName);
		return(false);
	}

	RiffWave fhd = {0};
	//memset(&fhd, 0, sizeof(fhd));
	fread((char *)&(fhd.riff), 8, 1, fpIn);

	if (memcmp(fhd.riff.name, "RIFF", 4) != 0)
	{
		printf("ReadWaveFile: File %s is not a valid .WAV file!\n", fileName);
		return(false);
	}

	uint32_t length = 0;
	size_t count = 0;

	do
	{
		count = fread(fhd.wave.name, 4, 1, fpIn);

		if(memcmp(fhd.wave.name, "WAVE", 4) == 0)
		{
			break;
		}

		fread((char*)&length, 4, 1, fpIn);
		fseek(fpIn, length, SEEK_CUR);

	}
	while( count > 0);

	do
	{
		count = fread(fhd.wave.fmt.name, 4, 1, fpIn);

		if(memcmp(fhd.wave.fmt.name, "fmt ", 4) == 0)
		{
			break;
		}

		fread((char*)&length, 4, 1, fpIn);
		fseek(fpIn, length, SEEK_CUR);

	}
	while (count > 0);

	fread((char*)&fhd.wave.fmt.length, 4, 1, fpIn);
	fread((char*)&fhd.wave.fmt.info, sizeof(fhd.wave.fmt.info), 1, fpIn);

	fseek(fpIn, fhd.wave.fmt.length - 16, SEEK_CUR);
	fread((char *)&fhd.wave.data, 8, 1, fpIn);

	while(memcmp(fhd.wave.data.name, "data", 4) != 0)
	{
		fseek(fpIn, fhd.wave.data.length, SEEK_CUR);
		fread((char *)&fhd.wave.data, 8, 1, fpIn);
	}

	/* get the data size */
	samplesPerSec = fhd.wave.fmt.info.nSamplesPerSec;
	bitsPerSample = fhd.wave.fmt.info.nBitsPerSample;
	channelsCount = fhd.wave.fmt.info.nChannels;
	samplesCount = (fhd.wave.data.length * 8) / (bitsPerSample * channelsCount);

	/* sampling interval in seconds: */
	//int delta = 1.0 / (float)samplesPerSec;
	//printf("interval = %fs\n", delta);

	printf("ReadWaveFile: File %s has %d %dbit samples\n", fileName, samplesCount, bitsPerSample);
	printf("ReadWaveFile: recorded at %d samples per second, ", samplesPerSec);
	printf((channelsCount == 2) ? "in Stereo.\n" : "%d channels.\n", channelsCount);
	printf("ReadWaveFile: Play duration: %6.2f seconds.\n", (float)samplesCount / (float)samplesPerSec);

	*pfSamples = new float *[channelsCount];
	for(int channelNumber = 0; channelNumber < channelsCount; ++channelNumber)
	{
		float *data;
		(*pfSamples)[channelNumber] = data = new float[(samplesCount + 1)];
		if (data == NULL) {
			printf("ReadWaveFile: Failed to allocate %d floats\n", samplesCount + 1);
			return(false);
		}
		for (int j = 0; j < (samplesCount + 1); j++) data[j] = 0.0;
	}

	auto bytesPerSam = bitsPerSample / 8;

	if(!bytesPerSam)
    {
        printf("ReadWaveFile: broken file\n");
        return false;
    }

	/* read wave samples, convert to floating point: */
	auto wavDataSize(channelsCount * samplesCount * bytesPerSam);
	*pSamples = new uint8_t[ wavDataSize ];

	if(!*pSamples)
	{
		printf("ReadWaveFile: Failed to allocate %d bytes\n", wavDataSize);

		return false;
	}

	fread(*pSamples, wavDataSize, 1, fpIn);

	switch(bitsPerSample)
	{
	case 8:
		for (int sampleNumber = 0; sampleNumber < samplesCount; sampleNumber++){
			int k;
			k = sampleNumber*channelsCount;
			for (int n = 0; n < channelsCount;n++)
			{
				(*pfSamples)[n][sampleNumber] = (float)((*pSamples)[k + n] - 127) / 256.0f;
			}
		}
		break;
	case 16:
		for (int sampleNumber = 0; sampleNumber < samplesCount; sampleNumber++){
			int k;
			k = sampleNumber*channelsCount;
			for (int n = 0; n < channelsCount; n++)
			{
				(*pfSamples)[n][sampleNumber] = (float)(((uint16_t *)*pSamples)[k + n]) / 32768.0f;
			}
		}
		break;
	case 32:
		for (int sampleNumber = 0; sampleNumber < samplesCount; sampleNumber++){
			int k;
			k = sampleNumber*channelsCount;
			for (int n = 0; n < channelsCount; n++)
			{
				(*pfSamples)[n][sampleNumber] = ((float *)*pSamples)[k + n];
			}
		}
		break;
	}

	fclose(fpIn);

	return true;
}

bool WriteWaveFileF(const char *fileName, int samplesPerSec, int channelsCount, int bitsPerSample, long samplesCount, float **pSamples)
{
	/* write wave samples: */
	RiffWave fhd;
	FILE *fpOut;

	SetupWaveHeader(&fhd, samplesPerSec, bitsPerSample, channelsCount, samplesCount);
	int bytesPerSample = bitsPerSample / 8;
    if (fopen_s(&fpOut, fileName, "wb") != 0 || !fpOut) return false;
	fwrite(&fhd, sizeof(fhd), 1, fpOut);

	char *buffer = new char[bytesPerSample*channelsCount*samplesCount];
	short *sSamBuf = (short *)buffer;
	float *fSamBuf = (float *)buffer;

	switch (bytesPerSample){
	case 1:
		for (int sampleNumber = 0; sampleNumber<samplesCount; sampleNumber++){
			int k;
			k = sampleNumber*channelsCount;
			for (int n = 0; n<channelsCount; n++) {
				float value = pSamples[n][sampleNumber];
				if (value > 1.0) value = 1.0;
				if (value < -1.0) value = -1.0;
				buffer[k + n] = (char)(value * 127.0);
			}
		}
		break;
	case 2:
		for (int sampleNumber = 0; sampleNumber<samplesCount; sampleNumber++){
			int k;
			k = sampleNumber*channelsCount;
			for (int n = 0; n<channelsCount; n++) {
				float value = pSamples[n][sampleNumber];
				if (value > 1.0) value = 1.0;
				if (value < -1.0) value = -1.0;
				sSamBuf[k+n] = (short)(value * 32767.0);
			}
		}
		break;
	case 4:
		for (int sampleNumber = 0; sampleNumber<samplesCount; sampleNumber++){
			int k;
			k = sampleNumber*channelsCount;
			for (int n = 0; n<channelsCount; n++) {
				fSamBuf[k + n] = pSamples[n][sampleNumber];
			}
		}
		break;
	default:
		return false;
		break;
	}


	fwrite(buffer, samplesCount*channelsCount * bytesPerSample, 1, fpOut);
	fclose(fpOut);

	return(0);

}

bool WriteWaveFileS(const char *fileName, int samplesPerSec, int channelsCount, int bitsPerSample, long samplesCount, short *pSamples)
{
	/* write wave samples: */
	RiffWave fhd;
	FILE *fpOut;

	SetupWaveHeader(&fhd, samplesPerSec, bitsPerSample, channelsCount, samplesCount);
	int bytesPerSample = bitsPerSample / 8;
	fopen_s(&fpOut,fileName, "wb");
	if (fpOut == NULL)
		return false;

	fwrite(&fhd, sizeof(fhd), 1, fpOut);

	char *buffer = new char[bytesPerSample*channelsCount*samplesCount];
	short *sSamBuf = (short *)buffer;
	float *fSamBuf = (float *)buffer;

	switch (bytesPerSample){
	case 1:
		for (int sampleNumber = 0; sampleNumber<samplesCount; sampleNumber++){
			int k;
			k = sampleNumber*channelsCount;
			for (int n = 0; n<channelsCount; n++) {
				short value = pSamples[k + n];
				buffer[k + n] = (short)(value >> 8);
			}
		}
		break;
	case 2:
		for (int sampleNumber = 0; sampleNumber<samplesCount; sampleNumber++){
			int k;
			k = sampleNumber*channelsCount;
			for (int n = 0; n<channelsCount; n++) {
				short value = pSamples[k + n];
				sSamBuf[k + n] = value;
			}
		}
		break;
	case 4:
		for (int sampleNumber = 0; sampleNumber<samplesCount; sampleNumber++){
			int k;
			k = sampleNumber*channelsCount;
			for (int n = 0; n<channelsCount; n++) {
				fSamBuf[k + n] = (float)pSamples[k + n] / 32767;
			}
		}
		break;
	default:
		return false;
		break;
	}


	fwrite(buffer, samplesCount*channelsCount * bytesPerSample, 1, fpOut);
	fclose(fpOut);

	return true;
}

#ifdef __cplusplus

#include <iostream>

bool WavContent::ReadWaveFile(const std::string & fileName)
{
	Reset();

	FILE *fpIn = nullptr;
	if((fopen_s(&fpIn, fileName.c_str(), "rb")) != 0)
	{
		std::cerr << "ReadWaveFile: Can't open " << fileName << std::endl;

		return false;
	}

	RiffWave fhd = {0};
	fread((char *)&(fhd.riff), 8, 1, fpIn);

	if(memcmp(fhd.riff.name, "RIFF", 4) != 0)
	{
		std::cerr << "ReadWaveFile: File " << fileName << " is not a valid .WAV file!" << std::endl;

		return false;
	}

	uint32_t length = 0;
	size_t count = 0;

	do
	{
		count = fread(fhd.wave.name, 4, 1, fpIn);

		if(memcmp(fhd.wave.name, "WAVE", 4) == 0)
		{
			break;
		}

		fread((char*)&length, 4, 1, fpIn);
		fseek(fpIn, length, SEEK_CUR);

	}
	while(count > 0);

	do
	{
		count = fread(fhd.wave.fmt.name, 4, 1, fpIn);

		if(memcmp(fhd.wave.fmt.name, "fmt ", 4) == 0)
		{
			break;
		}

		fread((char*)&length, 4, 1, fpIn);
		fseek(fpIn, length, SEEK_CUR);

	}
	while(count > 0);

	fread((char*)&fhd.wave.fmt.length, 4, 1, fpIn);
	fread((char*)&fhd.wave.fmt.info, sizeof(fhd.wave.fmt.info), 1, fpIn);

	fseek(fpIn, fhd.wave.fmt.length - 16, SEEK_CUR);
	fread((char *)&fhd.wave.data, 8, 1, fpIn);

	while(memcmp(fhd.wave.data.name, "data", 4) != 0)
	{
		fseek(fpIn, fhd.wave.data.length, SEEK_CUR);
		fread((char *)&fhd.wave.data, 8, 1, fpIn);
	}

	//
	if(fhd.wave.fmt.info.nBitsPerSample != 8 &&
	    fhd.wave.fmt.info.nBitsPerSample != 16 &&
		fhd.wave.fmt.info.nBitsPerSample != 32)
    {
        std::cerr << "Error: invalid bits per sample value" << std::endl;

        return false;
	}

	/* get the data size */
	auto samplesCount = (fhd.wave.data.length * 8) / (fhd.wave.fmt.info.nBitsPerSample * fhd.wave.fmt.info.nChannels);
	auto wavDataSize = fhd.wave.fmt.info.nChannels * samplesCount * fhd.wave.fmt.info.nBitsPerSample / 8;
	Data.resize(wavDataSize);

	if(Data.size() != wavDataSize)
	{
		std::cerr << "Error: could not allocate memory for wav file" << std::endl;

		return false;
	}

	/* get the data size */
	ChannelsCount = fhd.wave.fmt.info.nChannels;
	BitsPerSample = fhd.wave.fmt.info.nBitsPerSample;
	SamplesCount = (fhd.wave.data.length * 8) / (fhd.wave.fmt.info.nBitsPerSample * fhd.wave.fmt.info.nChannels);
	SamplesPerSecond = fhd.wave.fmt.info.nSamplesPerSec;

	/* sampling interval in seconds: */
	//int delta = 1.0 / (float)samplesPerSec;
	//printf("interval = %fs\n", delta);

	printf("File %s has %d %dbit samples\n", fileName.c_str(), SamplesCount, BitsPerSample);
	printf("Recorded at %d samples per second, ", SamplesPerSecond);
	printf((ChannelsCount == 2) ? "in Stereo.\n" : "%d channels.\n", ChannelsCount);
	std::cout << "Play duration: " << GetDuration().count() << std::endl;

	/* read wave samples, convert to floating point: */
	fread(&Data.front(), wavDataSize, 1, fpIn);

	fclose(fpIn);

	return Valid();
}

bool WavContent::Convert2Stereo16Bit()
{
	throw std::runtime_error("Error: not implemented!");

	/*if(2 == ChannelsCount && 16 == BitsPerSample)
	{
		return true;
	}

	/*if(ChannelsCount > 2)
	{
		std::cerr << "Error: not supported conversion!" << std::endl;

		return false;
	}* /
	std::vector<uint8_t> floatBuffer(sizeof(float) * 2 * SamplesCount /*+ 1* /, 0);

	switch(BitsPerSample)
	{
	case 8:
		for(int sampleNumber = 0; sampleNumber < SamplesCount; ++sampleNumber)
		{
			int k = sampleNumber * ChannelsCount;

			for(int channelNumber = 0; channelNumber < ChannelsCount; ++channelNumber)
			{
				floatBuffer.data()[channelNumber * sa ][sampleNumber] = (float)((*pSamples)[k + channelNumber] - 127) / 256.0f;
			}
		}

		break;

	case 16:
		for (int sampleNumber = 0; sampleNumber < samplesCount; sampleNumber++){
			int k;
			k = sampleNumber*channelsCount;
			for (int n = 0; n < channelsCount; n++)
			{
				(*pfSamples)[n][sampleNumber] = (float)(((uint16_t *)*pSamples)[k + n]) / 32768.0f;
			}
		}
		break;

	case 32:
		for (int sampleNumber = 0; sampleNumber < samplesCount; sampleNumber++){
			int k;
			k = sampleNumber*channelsCount;
			for (int n = 0; n < channelsCount; n++)
			{
				(*pfSamples)[n][sampleNumber] = ((float *)*pSamples)[k + n];
			}
		}
		break;
	}

	auto convertedData(2 * SamplesCount * sizeof(int16_t));

	try
	{
	    std::vector<uint8_t> convertedData(2 * SamplesCount * sizeof(int16_t));

		int16_t *pSBuf = (int16_t *)convertedData.begin();

		for(int sampleNumber = 0; sampleNumber < SamplesCount; ++sampleNumber)
        {
			int16_t channelNumber(0);

            pSBuf[2 * sampleNumber + 1] = pSBuf[2 * sampleNumber] = (int16_t)(32767 * Data[channelNumber * SamplesCount + sampleNumber]);

			if(ChannelsCount == 2)
            {
                pSBuf[2 * sampleNumber + 1] = (int16_t)(32767 * pSamples[1][sampleNumber]);
            }
        }
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << std::endl;

		return false;
	}
	*/

	return false;
}

#endif