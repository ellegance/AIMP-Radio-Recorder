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

AIMP_Communicator::AIMP_Communicator(){
	aimpRemoteAPINameLen = strlen(AIMPRemoteAccessClass) + 1;
	aimpWideCharRemoteAPINameptr.reset(new wchar_t[aimpRemoteAPINameLen]);
	mbstowcs(aimpWideCharRemoteAPINameptr.get(), AIMPRemoteAccessClass, aimpRemoteAPINameLen); //MultiByteToWideChar(CP_ACP, 0, charArray, -1, dst.get(), length); //also possible
	aimpHndlr = FindWindow(aimpWideCharRemoteAPINameptr.get(), NULL);
	DWORD err = GetLastError();
	aimpRunning = aimpHndlr ? true : false;
	GetUpdateCaptureStatus();
	GetUpdatePlayingStatus();
	fileInfoHndl = OpenFileMapping(FILE_MAP_READ, true, aimpWideCharRemoteAPINameptr.get());
	mappedFileInfo = static_cast<PAIMPRemoteFileInfo>(MapViewOfFile(fileInfoHndl, FILE_MAP_READ, 0, 0, AIMPRemoteAccessMapFileSize));
	captureCheckpoint = std::chrono::high_resolution_clock::now();
};

bool AIMP_Communicator::IsAIMPRunning() const{
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
	wchar_t* buf = (wchar_t*)mappedFileInfo;
	buf += sizeof(TAIMPRemoteFileInfo) / 2 + mappedFileInfo->AlbumLength + mappedFileInfo->ArtistLength + mappedFileInfo->DateLength
		+ mappedFileInfo->FileNameLength + mappedFileInfo->GenreLength; //you cannot concat this expr with the last cause of wrong shift on sizeof struct! TODO /2 change
	std::wstring wtrackName(buf, buf + mappedFileInfo->TitleLength);
	return wtrackName;
};

std::wstring AIMP_Communicator::GetArtistName() const{
	wchar_t* buf = (wchar_t*)mappedFileInfo;
	buf += sizeof(TAIMPRemoteFileInfo) / 2 + mappedFileInfo->AlbumLength;
	std::wstring wartistName(buf, buf + mappedFileInfo->ArtistLength);
	return wartistName;
};

bool AIMP_Communicator::IsRadioMode() const{
	wchar_t* buf = (wchar_t*)mappedFileInfo;
	buf += sizeof(TAIMPRemoteFileInfo) / 2 + mappedFileInfo->AlbumLength + mappedFileInfo->ArtistLength + mappedFileInfo->DateLength;
	std::wstring wfileName(buf, buf + mappedFileInfo->FileNameLength);
	//some checks if starts with "X://..." or "http://..."
	if (wfileName[0] == L'h' && wfileName[1] == L't' && wfileName[2] == L't' && wfileName[3] == L'p')
		return true;
	return false;
};

bool AIMP_Communicator::IsPlaying() const{
	return playingNow;
};

bool AIMP_Communicator::IsCapturing() const{
	return capturingNow;
};

void AIMP_Communicator::RegisterCallback(HWND hWnd) const{
	SendMessage(aimpHndlr, WM_AIMP_COMMAND, AIMP_RA_CMD_REGISTER_NOTIFY, (LPARAM)hWnd);
};

AIMP_Communicator::~AIMP_Communicator(){
	if (capturingNow)
		SendMessage(aimpHndlr, WM_AIMP_PROPERTY, AIMP_RA_PROPERTY_RADIOCAP | AIMP_RA_PROPVALUE_SET, static_cast<LPARAM>(0));
	UnmapViewOfFile(mappedFileInfo);
	CloseHandle(fileInfoHndl);
	//aimpHndlr isn't needed to be SendMessage(aimpHndlr, WM_CLOSE, 0, 0 )?
};