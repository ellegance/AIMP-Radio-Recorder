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
//
// Tested on SDK aimp_sdk_v3.60_build_1497
// Tested on AIMP v.3.60

#include "AIMP_Communicator.h"
#include "UI_MainWindow.h"
#include "Fav_Tracks_Handler.h"
#include "resource.h"

#define ERR_APP_SINGLE_INST  300
#define ERR_AIMP_NOT_RUNNING 301

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow){
	try {
		UNREFERENCED_PARAMETER(hPrevInstance); //macro for L4 warning C4100 avoid 
		UNREFERENCED_PARAMETER(lpCmdLine);
		HANDLE hMutex = OpenMutex(READ_CONTROL, 0, L"AIMP_TRACKRECORDER");
		if (!hMutex)
			hMutex = CreateMutex(0, 0, L"AIMP_TRACKRECORDER");
		else
			throw ERR_APP_SINGLE_INST;

		MSG message;
		AIMP_Communicator& aimpCommunicator = AIMP_Communicator::GetAIMP_CommunicatorInstance();
		if (!aimpCommunicator.IsAIMPRunning())
			throw ERR_AIMP_NOT_RUNNING;
		FavTracksMemoryModel tracksModel("D:\\tracklist.txt");
		MainWindow ui_window(L"AIMP RadioTrack recorder", hInstance, nCmdShow, &tracksModel, aimpCommunicator, L"Application", 100, 100, 361, 190);
		HWND windowHWND = ui_window.GetHWnd();
		aimpCommunicator.RegisterCallback(windowHWND);

		while (GetMessage(&message, NULL, 0, 0) > 0) {
			TranslateMessage(&message);
			DispatchMessage(&message);
			if (!aimpCommunicator.IsAIMPRunning())
				throw ERR_AIMP_NOT_RUNNING;
		}
		////a lot of CPU usage, but timed execution
		//while (true) {
		//	if (PeekMessage(&message, NULL, 0, 0, PM_REMOVE)) {
		//		TranslateMessage(&message);
		//		DispatchMessage(&message);
		//	}
		//	if (!aimpCommunicator.IsAIMPRunning())
		//		throw ERR_AIMP_NOT_RUNNING;
		//}
		ReleaseMutex(hMutex);
	}
	catch (int err_code) {
		std::wstring error_text;
		switch (err_code) {
		case ERR_AIMP_NOT_RUNNING:
			error_text = L"AIMP is not running now. Exiting";
			break;
		case ERR_APP_SINGLE_INST:
			error_text = L"AIMP Track Recorder is already running. Exiting";
			break;
		}
		MessageBox(NULL, error_text.c_str(), (LPCWSTR)L"AIMP Track Recorder error", MB_ICONWARNING | MB_OK);
	}
	return 0;
};