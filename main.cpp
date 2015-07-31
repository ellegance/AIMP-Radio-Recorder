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

//          WParam: AIMP_RA_NOTIFY_PROPERTY (Notification ID)
//          LParam: Property ID
//WndProc is wired through WNDCLASSEX which we register for our class
//Logic for update notifications: 1. Updated track name 2. Change playing mode 3. Radio/songs 4. Capture track 5. ...

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow){
	try {
		HANDLE hMutex = OpenMutex(MUTEX_ALL_ACCESS, 0, L"AIMP_TRACKRECORDER");
		if (!hMutex)
			hMutex = CreateMutex(0, 0, L"AIMP_TRACKRECORDER");
		else
			throw ERR_APP_SINGLE_INST;

		MSG message;

		AIMP_Communicator& aimpCommunicator = AIMP_Communicator::GetAIMP_CommunicatorInstance();
		if (!aimpCommunicator.IsAIMPRunning()) //todo: 1 more thread to track AIMP execution for throwing during execution
			throw ERR_AIMP_NOT_RUNNING;
		FavTracksMemoryModel tracksModel("D:\\tracklist.txt");
		MainWindow ui_window(L"AIMP RadioTrack recorder", hInstance, nCmdShow, &tracksModel, aimpCommunicator, L"Application", 100, 100, 361, 190);
		HWND windowHWND = ui_window.GetHWnd();
		aimpCommunicator.RegisterCallback(windowHWND);

		//BOOL WINAPI GetMessage(_Out_ LPMSG lpMsg, _In_opt_ HWND hWnd, _In_ UINT wMsgFilterMin, _In_ UINT wMsgFilterMax);
		while (GetMessage(&message, NULL, 0, 0)){
			TranslateMessage(&message);
			DispatchMessage(&message);
		}

		ReleaseMutex(hMutex);
	}
	catch (int err_code) {//todo: alertbox with error explanation
		return 0;
	}
	return 0;
};