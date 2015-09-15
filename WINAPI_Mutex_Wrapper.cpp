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

#include "WINAPI_Mutex_Wrapper.h"

WinAPIMutex::WinAPIMutex(_In_opt_ LPSECURITY_ATTRIBUTES lpMutexAttributes, _In_ BOOL bInitialOwner, _In_opt_ LPCWSTR mlpName) {
	mutexNameLngth = wcslen(mlpName) + 1;
	lpName = new TCHAR[mutexNameLngth];
	wcscpy_s(lpName, mutexNameLngth, mlpName);
	mutexHandle = OpenMutex(READ_CONTROL, NULL, mlpName);
	if (!mutexHandle) {
		mutexHandle = CreateMutex(lpMutexAttributes, bInitialOwner, mlpName);
		mutexAlreadyExisted = false;
		WaitForSingleObject(mutexHandle, INFINITE);
	}
	else
		mutexAlreadyExisted = true;
};

WinAPIMutex::~WinAPIMutex() {
	if (!mutexAlreadyExisted) {
		ReleaseMutex(mutexHandle);
		CloseHandle(mutexHandle);
	}
	delete[] lpName;
};

bool WinAPIMutex::MutexAlreadyExisted() const {
	return mutexAlreadyExisted;
};