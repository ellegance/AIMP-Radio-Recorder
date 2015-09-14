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

#define BUTTON_FAVOURITE_TRACK 1337
#define BUTTON_MANUALREC 1338
#define STATUS_BAR 1339
#define LABEL_ARTISTNAME 1340
#define LABEL_TRACKNAME 1341
#define LABEL_ARTIST 1342
#define LABEL_TRACK 1343
#define TOOLTIP_FAVTRACK 1344
#define TOOLTIP_MANUALREC 1345

void MainWindow::UpdateStatusBarAndButtonIconsInfo() {
	std::wstring tracksCaptured = std::to_wstring(_model->TracksRecorded());
	std::wstring tracksTotal = std::to_wstring(_model->GetNumberTracksInPlaylist());
	tracksCaptured += L" track(s) recorded";
	tracksTotal += L" total track(s)";
	SendMessage(statusBarHwnd, SB_SETTEXT, 0, (LPARAM)tracksCaptured.c_str());
	SendMessage(statusBarHwnd, SB_SETTEXT, 1, (LPARAM)tracksTotal.c_str());

	std::wstring playingMode;
	if (!aimpCommunicator.IsPlaying())
		playingMode += L"Stopped";
	else
		playingMode += aimpCommunicator.IsRadioMode() ? L"Radio " : L"Local file";

	if (aimpCommunicator.IsCapturing()) {
		playingMode += L"Recording";
		SendMessage(manualRecordSwitchButtonHwnd, BM_SETIMAGE, static_cast<WPARAM>(IMAGE_BITMAP), (LPARAM)hIconManualRecOn);
		}
	else
		SendMessage(manualRecordSwitchButtonHwnd, BM_SETIMAGE, static_cast<WPARAM>(IMAGE_BITMAP), (LPARAM)hIconManualRecOff);
	SendMessage(statusBarHwnd, SB_SETTEXT, 2, (LPARAM)playingMode.c_str());
};

void MainWindow::UpdateCurrentTrackInfo() {
	std::wstring artist = aimpCommunicator.GetArtistName();
	std::wstring trackName = aimpCommunicator.GetTrackName();
	artist.length() > 0 ? SetWindowTextW(artistNameLabelHwnd, artist.c_str()) : SetWindowTextW(artistNameLabelHwnd, L"Unknown Artist");
	trackName.length() > 0 ? SetWindowTextW(trackNameLabelHwnd, trackName.c_str()) : SetWindowTextW(trackNameLabelHwnd, L"Unknown Track"); 
};

void MainWindow::AIMPMessageHandler(WPARAM wparam, LPARAM lparam) {//on close action property?
	switch (wparam) {
	case AIMP_RA_NOTIFY_TRACK_INFO:{
		//to be continued...
		break;
	}
	case AIMP_RA_NOTIFY_TRACK_START:{//AIMP bug-feature #1: when you start radio manually this message sent twice!!!... //solution is inner timer ~<1 sec
		//AIMP bug-feature #2: when turn from radio to radio 2 calls AIMP_RA_PROPERTY_PLAYER_STATE && AIMP_RA_NOTIFY_TRACK_START !!!
		//get name of new track, check in model for it, if so, start capture
		
		if (aimpCommunicator.IsRadioMode()) {
			//lucky situation, when favourite track is recorded, check; or just AIMP user used recording
			if (aimpCommunicator.GetUpdateCaptureStatus() && !aimpCommunicator.IsAIMPbugfeatureManualRadioTurnedOn())
				aimpCommunicator.StopCapture();

			std::wstring artist = aimpCommunicator.GetArtistName();
			std::wstring trackname = aimpCommunicator.GetTrackName();

			if (_model->IsTrackInModel(trackname, artist) && !aimpCommunicator.IsAIMPbugfeatureManualRadioTurnedOn() && aimpCommunicator.IsPlaying()) {
				aimpCommunicator.StartCapture();
				_model->RemoveTrack(trackname, artist);
			}
		}
		break;
		}
	case AIMP_RA_NOTIFY_PROPERTY:{ //continuously receiving track position in msec's and volume/else events...
		switch (lparam) {
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
	UpdateStatusBarAndButtonIconsInfo();
};

HWND CreateToolTip(int toolID, HWND hDlg, PTSTR pszText)
{
	if (!toolID || !hDlg || !pszText)
	{
		return FALSE;
	}
	// Get the window of the tool.
	HWND hwndTool = GetDlgItem(hDlg, toolID);

	// Create the tooltip. g_hInst is the global instance handle.
	HWND hwndTip = CreateWindowEx(NULL, TOOLTIPS_CLASS, NULL,
		WS_POPUP | TTS_ALWAYSTIP | TTS_BALLOON,
		CW_USEDEFAULT, CW_USEDEFAULT,
		CW_USEDEFAULT, CW_USEDEFAULT,
		hDlg, NULL,
		GetModuleHandle(NULL), NULL);

	if (!hwndTool || !hwndTip)
	{
		return (HWND)NULL;
	}

	// Associate the tooltip with the tool.
	TOOLINFO toolInfo = { 0 };
	toolInfo.cbSize = sizeof(toolInfo);
	toolInfo.hwnd = hDlg;
	toolInfo.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
	toolInfo.uId = (UINT_PTR)hwndTool;
	toolInfo.lpszText = pszText;
	LRESULT AS = SendMessage(hwndTip, TTM_ADDTOOL, 0, (LPARAM)&toolInfo);

	return hwndTip;
}

LRESULT CALLBACK MainWindow::WindowMessageHandler(HWND hnd, UINT code, WPARAM wparam, LPARAM lparam) {
	switch (code) {
	case WM_AIMP_NOTIFY:
		AIMPMessageHandler(wparam, lparam);
		break;
	case WM_CREATE:{
		addFavouriteButtonHwnd = CreateWindowEx(NULL, L"BUTTON", NULL/*L"Add to favourite tracks"*/, WS_CHILD | WS_VISIBLE | BS_BITMAP | BS_FLAT,
			305, 26, 34, 34, hnd, (HMENU)BUTTON_FAVOURITE_TRACK, GetModuleHandle(NULL), NULL);
		manualRecordSwitchButtonHwnd = CreateWindowEx(NULL, L"BUTTON", NULL/*L"Add to favourite tracks"*/, WS_CHILD | WS_VISIBLE | BS_BITMAP | BS_FLAT,
			305, 86, 34, 34, hnd, (HMENU)BUTTON_MANUALREC, GetModuleHandle(NULL), NULL);
		HFONT hFont = CreateFont(14, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, TEXT("Tahoma"));
		artistLabelHwnd = CreateWindowEx(WS_EX_CLIENTEDGE, L"STATIC", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT, 10, 15, 100, 20, hnd, (HMENU)LABEL_ARTIST, NULL, NULL);
		artistNameLabelHwnd = CreateWindowEx(WS_EX_CLIENTEDGE, L"STATIC", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT /*| ES_READONLY//for editField*/, 10, 40, 290, 20, hnd, (HMENU)LABEL_ARTIST, GetModuleHandle(NULL), NULL);
		trackLabelHwnd = CreateWindowEx(WS_EX_CLIENTEDGE, L"STATIC", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT, 10, 75, 100, 20, hnd, (HMENU)LABEL_ARTIST, NULL, NULL);
		trackNameLabelHwnd = CreateWindowEx(WS_EX_CLIENTEDGE, L"STATIC", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT, 10, 100, 290, 20, hnd, (HMENU)LABEL_TRACKNAME, GetModuleHandle(NULL), NULL);
		SendMessage(artistNameLabelHwnd, WM_SETFONT, (WPARAM)hFont, TRUE);
		SendMessage(trackNameLabelHwnd, WM_SETFONT, (WPARAM)hFont, TRUE);
		SendMessage(artistLabelHwnd, WM_SETFONT, (WPARAM)hFont, TRUE);
		SendMessage(trackLabelHwnd, WM_SETFONT, (WPARAM)hFont, TRUE);
		SetWindowTextW(artistLabelHwnd, L"Artist:");
		SetWindowTextW(trackLabelHwnd, L"Title:");
		hIconAddFavourite = LoadBitmap(hinst, MAKEINTRESOURCE(IDB_BITMAP1));
		hIconManualRecOn = LoadBitmap(hinst, MAKEINTRESOURCE(IDB_BITMAP2));
		hIconManualRecOff = LoadBitmap(hinst, MAKEINTRESOURCE(IDB_BITMAP3));
		SendMessage(addFavouriteButtonHwnd, BM_SETIMAGE, static_cast<WPARAM>(IMAGE_BITMAP), (LPARAM)hIconAddFavourite);
		InitializeStatusBar(hnd);
		UpdateCurrentTrackInfo();
		UpdateStatusBarAndButtonIconsInfo();
		break;
	}
	case WM_COMMAND:{
		switch (LOWORD(wparam)){
		case BUTTON_FAVOURITE_TRACK:{
			if (aimpCommunicator.IsRadioMode())
				_model->AddTrackToModel(aimpCommunicator.GetTrackName(), aimpCommunicator.GetArtistName());
			break;
			}
		case BUTTON_MANUALREC: {
			if (aimpCommunicator.IsRadioMode())
				aimpCommunicator.IsCapturing() ? aimpCommunicator.StopCapture() : aimpCommunicator.StartCapture();
			break;
			}
		}
		break;
	}
	case WM_DESTROY:{
		aimpCommunicator.StopAIMPCommunicator();
		PostQuitMessage(EXIT_SUCCESS);
		return 0;
	}
	}
	return DefWindowProc(hnd, code, wparam, lparam);
};

void MainWindow::InitializeStatusBar(HWND hnd) {
	int cParts = 3; //sections of status-bar
	RECT rcClient;
	HLOCAL hloc;
	PINT paParts;
	int i, nWidth;

	// Create the status bar.
	statusBarHwnd = CreateWindowEx(NULL, STATUSCLASSNAME, (PCTSTR)NULL, /*SBARS_SIZEGRIP | */WS_CHILD | WS_VISIBLE,	0, 0, 0, 0,	hnd, (HMENU)STATUS_BAR, GetModuleHandle(NULL), NULL);  
	
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
	SendMessage(statusBarHwnd, SB_SETPARTS, (WPARAM)cParts, (LPARAM)paParts);
	// Free the array, and return.
	LocalUnlock(hloc);
	LocalFree(hloc);
};

LRESULT CALLBACK MainWindow::StaticWindowMessageHandler(HWND hnd, UINT code, WPARAM wparam, LPARAM lparam){
	//MainWindow* that = reinterpret_cast<MainWindow*>(GetWindowLongPtr(hnd, GWLP_USERDATA));
	//return that->WindowMessageHandler(hnd, code, wparam, lparam);

	if (code == WM_NCCREATE) {
		LPCREATESTRUCT cs = (LPCREATESTRUCT)lparam;
		SetWindowLongPtr(hnd, GWLP_USERDATA, (LONG_PTR)cs->lpCreateParams);
	}
	// Get a window pointer associated with this window
	MainWindow *w = (MainWindow *)GetWindowLongPtr(hnd, GWLP_USERDATA);
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
	wc.lpfnWndProc = StaticWindowMessageHandler;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = sizeof(MainWindow*); //size of extra per-window data
	wc.hInstance = hInst;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH));
	wc.lpszMenuName = menuName;
	wc.lpszClassName = szClassName;
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	if (!RegisterClassEx(&wc)) {
		wchar_t msg[100] = L"Cannot register class: ";
		lstrcatW(msg, szClassName);
		MessageBox(NULL, msg, L"Error", MB_OK);
		return;
	}

	mainWindowHwnd = CreateWindow(szClassName, windowName, windowStyle, x, y, width, height, hParent, static_cast<HMENU>(NULL), hInst, this); // this call initiate pWndProc() call

	if (!mainWindowHwnd) {
		wchar_t text[100] = L"Cannot create window: ";
		//strcat(text, (char*)windowName);
		MessageBox(NULL, text, L"Error", MB_OK);
		return;
	}
	ShowWindow(mainWindowHwnd, cmdShow);
};