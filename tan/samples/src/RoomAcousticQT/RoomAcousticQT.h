#pragma once

#include "../common/SimpleVRaudio.h"
#include <string>

#define MAX_DEVICES 10

class RoomAcousticQT
{
public:
	RoomAcousticQT();
	virtual ~RoomAcousticQT();

	void initialize();															// Initialize the Room Acoustic
	int start();																// Start the demo
	void stop();																// Stop the demo

	void loadConfiguration(const std::string& xmlfilename);						// Load the configuration from the xml file
	void saveConfiguraiton(const std::string& xmlfilename);						// Save all the configruation in xml file named by the parameter

	int addSoundSource(const std::string& sourcename);							// Add a sound source in the audio engine
	bool removeSoundSource(const std::string& sourcename);						// Remove a sound source in the audio engine, return true if success
	bool removeSoundSource(int id);
	int  findSoundSource(const std::string& sourcename);						// Given the sound source name, find the corresponding soundsource ID,

	bool isInsideRoom(float x, float y, float z);								// determine if a point is inside the room or not.
																				// return -1 if not found

	float getReverbTime(float final_db, int* nreflections);						// Based on the room definition and given db, estimate the reverb time.
																				// Number of reflection will be returned in parameters

	float getConvolutionTime();													// Based on the convolution length, calculate the convoltion time
	float getBufferTime();														// Based on the buffer length, calculate the buffer tiem
	std::vector<std::string> getCPUConvMethod() const;							// Get the name of the supported CPU convolution method
	std::vector<std::string> getGPUConvMethod() const;							// Get the name of the supported GPU convolution method
	amf::TAN_CONVOLUTION_METHOD getConvMethodFlag(const std::string& _name);	// Convert a convolution method's name in to internal flag that can be used in runtime
	/*Run time - these function should be used only when engine is running*/
	void updateAllSoundSourcesPosition();										// update all the sound source position
	void updateSoundSourcePosition(int index);									// update the sound source position
	void updateListenerPosition();												// update the listener position
	void updateRoomDimention();
	void updateRoomDamping();
	AmdTrueAudioVR* getAMDTrueAudioVR();
	TANConverterPtr getTANConverter();

	void UpdateSoundSourcesPositions();
	
private:
	void initializeEnvironment();												// Initialize TAN DLL
	void initializeAudioEngine();												// Initialize TAN Audio3D Engine
	
	void initializeRoom();														// Initialize TAN Room definition
	void initializeConvolution();
	void initializeListener();													// Initialize TAN listener profile
	void initializeAudioPosition(int index);

	void enumDevices();															// Initialize TAN device (Convolution, FFT, etc.)

//todo: make accessors
public:
	std::string mTANDLLPath;
	std::string mLogPath;
	std::string mConfigFileName;

	std::string mWavFileNames[MAX_SOURCES];
	int m_iNumOfWavFile = 0;

	Audio3D* m_pAudioEngine;											// Pointer to the main audio3d engine
	RoomDefinition m_RoomDefinition;									// Roombox definition, contains damping and dimension
	StereoListener m_Listener;											// Listener configuration
	int m_iHeadAutoSpin = 0;
	
	/*Sound Source*/
	MonoSource m_SoundSources[MAX_SOURCES];								// All of the sound sources
	bool mSoundSourceEnable[MAX_SOURCES];								// sound sources' enable
	bool mSrcTrackHead[MAX_SOURCES];
	bool mSrc1EnableMic = false;

	/*Device*/
	int mCPUDevicesCount = 0;												// Device count
	std::string mCPUDevicesNames[MAX_DEVICES];								// Device names
	int mGPUDevicesCount = 0;												// Device count
	std::string mGPUDevicesNames[MAX_DEVICES];
	
	/*Convolution*/
	amf::TAN_CONVOLUTION_METHOD m_eConvolutionMethod =					// TAN Convolution method
		amf::TAN_CONVOLUTION_METHOD_FFT_OVERLAP_ADD;
	int m_iConvolutionLength = 0;
	int m_iBufferSize = 0;

	bool mConvolutionOverCL = false;
	bool mConvolutionOverGPU = false;
	int mConvolutionDeviceIndex = 0;
	int mConvolutionPriority = 0;
	int mConvolutionCUCount = 0;

	/*Room*/
	bool mRoomOverCL = false;
	bool mRoomOverGPU = false;
	int mRoomDeviceIndex = 0;
	int mRoomPriority = 0;
	int mRoomCUCount = 0;

	std::string mPlayerName;
};