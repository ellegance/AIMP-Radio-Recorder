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

#include "UI_MainWindow.h"
#include "resource.h"

#define FAVOURITE_TRACK 1337
#define STATUS_BAR 1338
#define EDIT_FIELD_ARTIST 1339
#define EDIT_FIELD_TRACKNAME 1340

void MainWindow::UpdateStatusBarInfo(){
	std::wstring tracksCaptured = std::to_wstring(_model->TracksRecorded());
	std::wstring tracksTotal = std::to_wstring(_model->GetNumberTracksInPlaylist());
	tracksCaptured += L" track(s) recorded";
	tracksTotal += L" total track(s)";
	SendMessage(MainWindowstatusBarhWnd, SB_SETTEXT, 0, (LPARAM)tracksCaptured.c_str());
	SendMessage(MainWindowstatusBarhWnd, SB_SETTEXT, 1, (LPARAM)tracksTotal.c_str());

	std::wstring playingMode;
	if (!aimpCommunicator.IsPlaying())
		playingMode += L"Stopped";
	else
		playingMode += aimpCommunicator.IsRadioMode() ? L"Radio " : L"Local file";

	if (aimpCommunicator.IsCapturing())
		playingMode += L"Recording";
	SendMessage(MainWindowstatusBarhWnd, SB_SETTEXT, 2, (LPARAM)&playingMode[0]);
};

void MainWindow::UpdateCurrentTrackInfo(){
	std::wstring artist = aimpCommunicator.GetArtistName();
	std::wstring trackName = aimpCommunicator.GetTrackName();
	SetWindowTextW(MainWindowEditFieldhWnd, artist.c_str());
	SetWindowTextW(MainWindowEditField2hWnd, trackName.c_str());
};

void MainWindow::AIMPMessageHandler(WPARAM wparam, LPARAM lparam){ //on close action property?
	switch (wparam){
	case AIMP_RA_NOTIFY_TRACK_INFO:{
		//to be continued...
		break;
	}
	case AIMP_RA_NOTIFY_TRACK_START:{  //AIMP bug-feature #1: when you start radio manually this message sent twice!!!... //solution is inner timer ~<1 sec
		//AIMP bug-feature #2: when turn from radio to radio 2 calls AIMP_RA_PROPERTY_PLAYER_STATE && AIMP_RA_NOTIFY_TRACK_START !!!
		//get name of new track, check in model for it, if so, start capture
		
		if (aimpCommunicator.IsRadioMode()){
			//lucky situation, when favourite track is recorded, check; or just AIMP user used recording
			if (aimpCommunicator.GetUpdateCaptureStatus() && !aimpCommunicator.IsAIMPbugfeatureManualRadioTurnedOn())
				aimpCommunicator.StopCapture();

			std::wstring artist = aimpCommunicator.GetArtistName();
			std::wstring trackname = aimpCommunicator.GetTrackName();

			if (_model->IsTrackInModel(trackname, artist) && !aimpCommunicator.IsAIMPbugfeatureManualRadioTurnedOn() && aimpCommunicator.IsPlaying()){
				aimpCommunicator.StartCapture();
				_model->RemoveTrackFromModel(trackname, artist);
			}
		}
		break;
		}
	case AIMP_RA_NOTIFY_PROPERTY:{ //continuously receiving track position in msec's and volume/else events...
		switch (lparam){
			case AIMP_RA_PROPERTY_PLAYER_STATE:
				aimpCommunicator.GetUpdatePlayingStatus();
				break;
			case AIMP_RA_PROPERTY_RADIOCAP: //when you stop AIMP (during recording), this property go first, then AIMP_RA_PROPERTY_PLAYER_STATE; the same is on close AIMP
				aimpCommunicator.GetUpdateCaptureStatus(true);
				break;
		}
		break;
		}
	}
	UpdateCurrentTrackInfo();
	UpdateStatusBarInfo();
};

LRESULT CALLBACK MainWindow::WindowMessageHandler(HWND hnd, UINT code, WPARAM wparam, LPARAM lparam){
	switch (code){
	case WM_AIMP_NOTIFY:
		AIMPMessageHandler(wparam, lparam);
		break;
	case WM_CREATE:{
		MainWindowbuttonhWnd = CreateWindowEx(NULL, L"BUTTON", NULL/*L"Add to favourite tracks"*/, WS_CHILD | WS_VISIBLE | BS_BITMAP,
			305, 38, 34, 34, hnd, (HMENU)FAVOURITE_TRACK, GetModuleHandle(NULL), NULL);
		MainWindowEditFieldhWnd = CreateWindowEx(NULL, L"EDIT", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER, 15, 30, 275, 20, hnd, (HMENU)EDIT_FIELD_ARTIST, GetModuleHandle(NULL), NULL);
		MainWindowEditField2hWnd = CreateWindowEx(NULL, L"EDIT", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER, 15, 60, 275, 20, hnd, (HMENU)EDIT_FIELD_TRACKNAME, GetModuleHandle(NULL), NULL);
		HBITMAP hbit = LoadBitmap(hinst, MAKEINTRESOURCE(IDB_BITMAP1));
		SendMessage(MainWindowbuttonhWnd, BM_SETIMAGE, static_cast<WPARAM>(IMAGE_BITMAP), (LPARAM)hbit);
		InitializeStatusBar(hnd); //Wired to AIMP - Time @track@ / No radio - Record indication/Greyed	
		UpdateStatusBarInfo();
		break;
	}
	case WM_COMMAND:{
		switch (LOWORD(wparam)){
		case FAVOURITE_TRACK:{
			if (aimpCommunicator.IsRadioMode())
				_model->AddTrackToModel(aimpCommunicator.GetTrackName(), aimpCommunicator.GetArtistName());
			break;
			}
		}
		break;
	}
	case WM_DESTROY:{
		PostQuitMessage(EXIT_SUCCESS);
		return 0;
	}
	}
	return DefWindowProc(hnd, code, wparam, lparam);
};



void MainWindow::InitializeStatusBar(HWND hnd){

	int cParts = 3; //sections of status-bar
	RECT rcClient;
	HLOCAL hloc;
	PINT paParts;
	int i, nWidth;

	// Ensure that the common control DLL is loaded.
	//InitCommonControls();

	// Create the status bar.
	MainWindowstatusBarhWnd = CreateWindowEx(NULL, STATUSCLASSNAME, (PCTSTR)NULL, /*SBARS_SIZEGRIP | */WS_CHILD | WS_VISIBLE,	0, 0, 0, 0,	hnd, (HMENU)STATUS_BAR, GetModuleHandle(NULL), NULL);  
	
	// Get the coordinates of the parent window's client area.
	GetClientRect(hnd, &rcClient);

	// Allocate an array for holding the right edge coordinates.
	hloc = LocalAlloc(LHND, sizeof(int) * cParts);
	paParts = (PINT)LocalLock(hloc);

	// Calculate the right edge coordinate for each part, and
	// copy the coordinates to the array.
	nWidth = rcClient.right / cParts;
	int rightEdge = nWidth;
	for (i = 0; i < cParts; i++) {
		paParts[i] = rightEdge;
		rightEdge += nWidth;
	}

	// Tell the status bar to create the window parts.
	SendMessage(MainWindowstatusBarhWnd, SB_SETPARTS, (WPARAM)cParts, (LPARAM)paParts);
	// Free the array, and return.
	LocalUnlock(hloc);
	LocalFree(hloc);
};


LRESULT CALLBACK MainWindow::StaticWindowMessageHandler(HWND hnd, UINT code, WPARAM wparam, LPARAM lparam){
	//MainWindow* that = reinterpret_cast<MainWindow*>(GetWindowLongPtr(hnd, GWLP_USERDATA));
	//return that->WindowMessageHandler(hnd, code, wparam, lparam);

	if (code == WM_NCCREATE) {
		LPCREATESTRUCT cs = (LPCREATESTRUCT)lparam;
		SetWindowLong(hnd, GWLP_USERDATA, (long)cs->lpCreateParams);
	}
	// Get a window pointer associated with this window
	MainWindow *w = (MainWindow *)GetWindowLong(hnd, GWLP_USERDATA);
	// Always check the return value, as WndProc will be called before it is set
	if (w) {
		// Redirect messages to the window procedure of the associated window, if we have one
		return w->WindowMessageHandler(hnd, code, wparam, lparam);
	}
	else {
		// Use default handling if the window is not associated yet
		return DefWindowProc(hnd, code, wparam, lparam);
	}

};


//void MainWindow::statusBarSetText(const std::wstring& txt, size_t section){
//	SendMessage(MainWindowstatusBarhWnd, SB_SETTEXT, section, (LPARAM)&txt[0]/*(LPARAM)L"OKOK"*/);
//};

//MainWindow ui_window(L"AIMP RadioTrack recorder", hInstance, nCmdShow, L"Application", 100, 100, 363, 190, new FavTracksMemoryModel("C:\\tracklist.txt", 18));
/*
MainWindow(LPCWSTR windowName,
HINSTANCE hInst,
int cmdShow,
LPCTSTR menuName = NULL,
int x = CW_USEDEFAULT, int y = 0,
int width = CW_USEDEFAULT, int height = 0,
UINT classStyle = CS_HREDRAW | CS_VREDRAW,
DWORD windowStyle = WS_OVERLAPPED | WS_MINIMIZEBOX | WS_SYSMENU,
HWND hParent = NULL,
FavTracksMemoryModel* model);
*/
MainWindow::MainWindow(LPCWSTR windowName,
	HINSTANCE hInst,
	int cmdShow,
	FavTracksMemoryModel* model,
	AIMP_Communicator& aimpCommInstance,
	LPCTSTR menuName,
	int x,
	int y,
	int width,
	int height,
	UINT classStyle,
	DWORD windowStyle,
	HWND hParent):hinst(hInst),
				 _model(model),
				 aimpCommunicator(aimpCommInstance)
{
	wchar_t szClassName[] = L"MainWindowClass";
	wc.cbSize = sizeof(wc);
	wc.style = classStyle;
	wc.lpfnWndProc = StaticWindowMessageHandler; //LRESULT(WINAPI *pWndProc)(HWND, UINT, WPARAM, LPARAM)
	wc.cbClsExtra = 0;
	wc.cbWndExtra = sizeof(MainWindow*); //size of extra per-window data
	wc.hInstance = hInst;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH));
	wc.lpszMenuName = menuName;
	wc.lpszClassName = szClassName;
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	//DWORD err = GetLastError();

	//aimpCommunicator = AIMP_Communicator::GetAIMP_CommunicatorInstance(); //what is the difference beetween this and :aimpCommunicator(aimpCommInstance) ? the last is optional initialization list
															//~95% answer is that this isn't ref assigning, its just object operator=, you can assign reference only in
															//constructor's base initializer list
	if (!RegisterClassEx(&wc)) {
		wchar_t msg[100] = L"Cannot register class: ";
		lstrcatW(msg, szClassName);
		MessageBox(NULL, msg, L"Error", MB_OK);
		return;
	}
	/*
	HWND WINAPI CreateWindow(
	_In_opt_ LPCTSTR   lpClassName,
	_In_opt_ LPCTSTR   lpWindowName,
	_In_     DWORD     dwStyle,
	_In_     int       x,
	_In_     int       y,
	_In_     int       nWidth,
	_In_     int       nHeight,
	_In_opt_ HWND      hWndParent,
	_In_opt_ HMENU     hMenu,
	_In_opt_ HINSTANCE hInstance,
	_In_opt_ LPVOID    lpParam
	);*/
	//SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)this);
	//CreateWindow(szClassName, windowName, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 500, 500, HWND_DESKTOP, NULL, GetModuleHandle(NULL), NULL);
	hWnd = CreateWindow(szClassName, windowName, windowStyle, x, y, width, height, hParent, static_cast<HMENU>(NULL), hInst, this); // this call initiate pWndProc() call
	//SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)&StaticWindowMessageHandler);

	if (!hWnd) {
		wchar_t text[100] = L"Cannot create window: ";
		//strcat(text, (char*)windowName);
		MessageBox(NULL, text, L"Error", MB_OK);
		return;
	}
	ShowWindow(hWnd, cmdShow);
};
