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

#ifndef WINAPI_EVENT_WRAPPER_H_
#define WINAPI_EVENT_WRAPPER_H_

#include <string>
#include <Windows.h>
//#include <tchar.h>

//RAII ready WinAPI mutex trivial wrapper, {} scope
//Mutex is used as OS Flag beetween processes to disable multiple run
class WinAPIMutex{
public:
	WinAPIMutex(_In_opt_ LPSECURITY_ATTRIBUTES lpMutexAttributes, _In_ BOOL bInitialOwner, _In_opt_ LPCWSTR mlpName);
	~WinAPIMutex();
	bool MutexAlreadyExisted() const;
private:
	HANDLE mutexHandle;
	BOOL manualReset;
	BOOL initialState;
	LPWSTR lpName;
	size_t mutexNameLngth;
	bool mutexAlreadyExisted;
};

#endif