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

#include "AIMP_Communicator.h"

AIMP_Communicator::AIMP_Communicator(): aimpTrackThread(){
	aimpRemoteAPINameLen = strlen(AIMPRemoteAccessClass) + 1;
	aimpWideCharRemoteAPINameptr.reset(new wchar_t[aimpRemoteAPINameLen]);
	size_t result;
	mbstowcs_s(&result, aimpWideCharRemoteAPINameptr.get(), aimpRemoteAPINameLen, &(AIMPRemoteAccessClass[0]), strlen(AIMPRemoteAccessClass));
	aimpHndlr = FindWindow(aimpWideCharRemoteAPINameptr.get(), NULL);
	aimpRunning = aimpHndlr ? true : false;
	GetUpdateCaptureStatus();
	GetUpdatePlayingStatus();
	fileInfoHndl = OpenFileMapping(FILE_MAP_READ, true, aimpWideCharRemoteAPINameptr.get());
	mappedFilePtr = static_cast<wchar_t const *>(MapViewOfFile(fileInfoHndl, FILE_MAP_READ, 0, 0, AIMPRemoteAccessMapFileSize));
	GetWindowThreadProcessId(aimpHndlr, &aimpProcId);
	aimpProcHndl = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, aimpProcId);
	aimpTrackRecorderShutdown = false;
	aimpTrackThread = std::thread(&AIMP_Communicator::aimpContinuousRunCheck, this);
	captureCheckpoint = std::chrono::high_resolution_clock::now();
};

void AIMP_Communicator::aimpContinuousRunCheck() {
	for (;;) {
		Sleep(500);
		GetExitCodeProcess(aimpProcHndl, &aimpExitCode);
		std::lock_guard<std::mutex> lockAimp(aimpRunFlagMtx);
		aimpRunning = aimpExitCode == 259 ? true : false;
		std::lock_guard<std::mutex> lockAimpTrackRecorder(aimpTrackRecorderShutdownMtx);
		if (aimpExitCode != 259 || aimpTrackRecorderShutdown)
			return;
	}
};

void AIMP_Communicator::StopAIMPCommunicator() {
	std::lock_guard<std::mutex> lockAimpTrackRecorder(aimpTrackRecorderShutdownMtx);
	aimpTrackRecorderShutdown = true;
};

bool AIMP_Communicator::IsAIMPRunning() const{
	std::lock_guard<std::mutex> lock(aimpRunFlagMtx);
	return aimpRunning;
};

AIMP_Communicator& AIMP_Communicator::GetAIMP_CommunicatorInstance(){
	static AIMP_Communicator instance; //Meyer's Singleton
	return instance;
};

void AIMP_Communicator::StartCapture(){
	SendMessage(aimpHndlr, WM_AIMP_PROPERTY, AIMP_RA_PROPERTY_RADIOCAP | AIMP_RA_PROPVALUE_SET, static_cast<LPARAM>(1));
    capturingNow = true;
	captureCheckpoint = std::chrono::high_resolution_clock::now();
};

void AIMP_Communicator::StopCapture(){
	SendMessage(aimpHndlr, WM_AIMP_PROPERTY, AIMP_RA_PROPERTY_RADIOCAP | AIMP_RA_PROPVALUE_SET, static_cast<LPARAM>(0));
	capturingNow = false;
};

//AIMP seems to have bug with wrong same track_start_notification on radio manual start http://www.aimp.ru/forum/index.php?topic=49406
bool AIMP_Communicator::IsAIMPbugfeatureManualRadioTurnedOn() const{ //not reliable solution, but intuitively understandable. It's just radio recorder, not quantitative trading ;)
	std::chrono::duration<double> elapsed_seconds = std::chrono::high_resolution_clock::now() - captureCheckpoint;
	auto time_diff = elapsed_seconds.count();
	return time_diff < 1.0;
};

bool AIMP_Communicator::GetUpdateCaptureStatus(bool initiateTimeReset) {
	LRESULT captureStatus = SendMessage(aimpHndlr, WM_AIMP_PROPERTY, AIMP_RA_PROPERTY_RADIOCAP | AIMP_RA_PROPVALUE_GET, NULL);
	capturingNow = (captureStatus != 0);
	if (capturingNow && initiateTimeReset)
		captureCheckpoint = std::chrono::high_resolution_clock::now();
	return capturingNow;
};

bool AIMP_Communicator::GetUpdatePlayingStatus() const{
	LRESULT playingStatus = SendMessage(aimpHndlr, WM_AIMP_PROPERTY, AIMP_RA_PROPERTY_PLAYER_STATE | AIMP_RA_PROPVALUE_GET, NULL);
	playingNow = (playingStatus == 2);
	return playingNow;
};

std::wstring AIMP_Communicator::GetTrackName() const{
	TAIMPRemoteFileInfo const * mappedFileInfo = (TAIMPRemoteFileInfo const *)mappedFilePtr;
	size_t name_begin_shift = sizeof(TAIMPRemoteFileInfo) / sizeof(wchar_t) + mappedFileInfo->AlbumLength + mappedFileInfo->ArtistLength 
							  + mappedFileInfo->DateLength + mappedFileInfo->FileNameLength + mappedFileInfo->GenreLength;
	size_t name_end_shift = name_begin_shift + mappedFileInfo->TitleLength;
	std::wstring wtrackName(mappedFilePtr + name_begin_shift, mappedFilePtr + name_end_shift);
	return wtrackName;
};

std::wstring AIMP_Communicator::GetArtistName() const{
	TAIMPRemoteFileInfo const * mappedFileInfo = (TAIMPRemoteFileInfo const *)mappedFilePtr;
	size_t name_begin_shift = sizeof(TAIMPRemoteFileInfo) / sizeof(wchar_t) + mappedFileInfo->AlbumLength;
	size_t name_end_shift = name_begin_shift + mappedFileInfo->ArtistLength;
	std::wstring wartistName(mappedFilePtr + name_begin_shift, mappedFilePtr + name_end_shift);
	return wartistName;
};

bool AIMP_Communicator::IsRadioMode() const{
	TAIMPRemoteFileInfo const * mappedFileInfo = (TAIMPRemoteFileInfo const *)mappedFilePtr;
	size_t name_begin_shift = sizeof(TAIMPRemoteFileInfo) / sizeof(wchar_t) + mappedFileInfo->AlbumLength + mappedFileInfo->ArtistLength + mappedFileInfo->DateLength;
	size_t name_end_shift = name_begin_shift + mappedFileInfo->FileNameLength;
	std::wstring wfileName(mappedFilePtr + name_begin_shift, mappedFilePtr + name_end_shift);
	if (wfileName.compare(0, 4, L"http") == 0)
		return true;
	return false;
};

bool AIMP_Communicator::IsPlaying() const{
	return playingNow;
};

bool AIMP_Communicator::IsCapturing() const{
	return capturingNow;
};

void AIMP_Communicator::RegisterCallback(HWND hWnd){
	callbackRegistrant = hWnd;
	SendMessage(aimpHndlr, WM_AIMP_COMMAND, AIMP_RA_CMD_REGISTER_NOTIFY, (LPARAM)hWnd);
};

AIMP_Communicator::~AIMP_Communicator(){
	if (capturingNow)
		SendMessage(aimpHndlr, WM_AIMP_PROPERTY, AIMP_RA_PROPERTY_RADIOCAP | AIMP_RA_PROPVALUE_SET, static_cast<LPARAM>(0));
	SendMessage(aimpHndlr, WM_AIMP_COMMAND, AIMP_RA_CMD_UNREGISTER_NOTIFY, (LPARAM)callbackRegistrant);
	UnmapViewOfFile(mappedFilePtr);
	CloseHandle(fileInfoHndl);
	aimpTrackThread.join();//exception non-safe
};