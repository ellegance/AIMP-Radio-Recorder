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
#include "Fav_Tracks_Handler.h"

#include <Windows.h>

#include <commctrl.h>

class MainWindow {
public:
	MainWindow(LPCWSTR windowName,
		HINSTANCE hInst,
		int cmdShow,
		FavTracksMemoryModel* model,
		AIMP_Communicator& aimpCommInstance,
		LPCTSTR menuName = NULL,
		int x = CW_USEDEFAULT, int y = 0,
		int width = CW_USEDEFAULT, int height = 0,
		UINT classStyle = CS_HREDRAW | CS_VREDRAW,
		DWORD windowStyle = WS_OVERLAPPED | WS_MINIMIZEBOX | WS_SYSMENU,
		HWND hParent = NULL);
	HWND GetHWnd() { return mainWindowHwnd; };
	void InitializeStatusBar(HWND hnd);

private:
	static LRESULT CALLBACK StaticWindowMessageHandler(HWND hnd, UINT code, WPARAM wparam, LPARAM lparam);
	//because each class non-static method actually gets hidden "this" as a parameter
	//works like stub for calling WindowMessageHandler()

	LRESULT CALLBACK WindowMessageHandler(HWND hnd, UINT code, WPARAM wparam, LPARAM lparam);
//	void statusBarSetText(const std::wstring& txt, size_t section);
	void UpdateStatusBarAndButtonIconsInfo();
	void UpdateCurrentTrackInfo();
	void AIMPMessageHandler(WPARAM wparam, LPARAM lparam);

	HWND mainWindowHwnd;
	HWND statusBarHwnd;
	HWND addFavouriteButtonHwnd;
	HBITMAP hIconAddFavourite;
	HBITMAP hIconManualRecOn;
	HBITMAP hIconManualRecOff;
	HWND manualRecordSwitchButtonHwnd;
	HWND artistNameLabelHwnd;
	HWND trackNameLabelHwnd;
	HWND artistLabelHwnd;
	HWND trackLabelHwnd;
	HWND hwndTip;
	WNDCLASSEX wc;
	HINSTANCE hinst;
	FavTracksMemoryModel* _model;
	AIMP_Communicator& aimpCommunicator; //singleton reference... seems to be faster then frequent GetAIMP_CommunicatorInstance() calls
};