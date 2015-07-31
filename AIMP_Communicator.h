// Copyright 2015 Viktor Chernov All Rights Reserved.
//
// AIMP RadioTrack Recorder
//
// Licensed under the Creative Commons Attribution NonCommercial Share-Alike (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// https://creativecommons.org/licenses/by-nc-sa/3.0/
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef AIMP_COMMUNICATOR_H_
#define AIMP_COMMUNICATOR_H_

#include <apiRemote.h>

#include <chrono>
#include <memory>
#include <string>


//Keep all AIMP handles
//Implements AIMP communication and maintains AIMP states, property get/set
//Works with shared memory-mapped file
//NB: call logic of AIMP notification messages is part of mainwindow callback AIMPMessageHandler.
class AIMP_Communicator{
private:
	AIMP_Communicator();
	AIMP_Communicator(AIMP_Communicator&) = delete;
	AIMP_Communicator& operator=(AIMP_Communicator&) = delete;
	~AIMP_Communicator();
	size_t aimpRemoteAPINameLen;
	std::unique_ptr<wchar_t[]> aimpWideCharRemoteAPINameptr;
	HWND aimpHndlr;
	HANDLE fileInfoHndl;
	PAIMPRemoteFileInfo mappedFileInfo;
	mutable bool capturingNow;
	mutable bool playingNow;
	bool aimpRunning;
	std::chrono::time_point<std::chrono::high_resolution_clock> captureCheckpoint;
public:
	static AIMP_Communicator& GetAIMP_CommunicatorInstance();
	void StartCapture();
	void StopCapture();
	bool GetUpdateCaptureStatus(bool initiateTimeReset = false);
	bool GetUpdatePlayingStatus() const;
	std::wstring GetTrackName() const;
	std::wstring GetArtistName() const;
	bool IsRadioMode() const;
	bool IsPlaying() const;
	bool IsCapturing() const;
	bool IsAIMPRunning() const;
	void RegisterCallback(HWND hWnd) const;
	bool IsAIMPbugfeatureManualRadioTurnedOn() const;
};

#endif