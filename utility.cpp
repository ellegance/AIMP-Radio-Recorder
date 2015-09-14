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

#include "utility.h"

//experimental parsing until first " - " is found
std::pair<std::string, std::string> getArtistAndTrackName(std::string& song) {
	size_t delimeter = song.find(" - ");
	return std::pair<std::string, std::string>(std::string(song, 0, delimeter), std::string(song, delimeter+3, song.length()));
};

void unifiedWstringfromWstring(std::wstring& uniWstr, const std::wstring& sourceWStr, size_t begin, size_t end) {
	for (size_t idx = begin; idx < end; idx++){
		if (iswalpha(sourceWStr[idx]))
			uniWstr += towupper(sourceWStr[idx]);
		else if (iswdigit(sourceWStr[idx]))
			uniWstr += sourceWStr[idx];
	}
};

void unifiedWstringfromWstring(std::wstring& uniWstr, const std::wstring& /*sourceWStr*/, std::wstring::iterator begin, std::wstring::iterator end) { //avoid warning C4100
	for (auto iter = begin; iter != end; iter++) {
		if (iswalpha(*iter))
			uniWstr += towupper(*iter);
		else if (iswdigit(*iter))
			uniWstr += *iter;
	}
};

void unifiedWstringfromString(std::wstring& uniWstr, const std::string& sourceStr, size_t begin, size_t end) {
	for (size_t idx = begin; idx < end; idx++) {
		if (isalpha(sourceStr[idx]))
			uniWstr += towupper(sourceStr[idx]);
		else if (isdigit(sourceStr[idx]))
			uniWstr += sourceStr[idx];
	}
};

size_t getSongHash(std::string& song) {
	std::wstring wsong;
	unifiedWstringfromString(wsong, song, 0, song.length());
	std::hash<std::wstring> wstrhash;
	return wstrhash(wsong);
};

size_t getSongHash(wchar_t* song_chars) {
	std::wstring wsong(song_chars);
	std::hash<std::wstring> wstrhash;
	return wstrhash(wsong);
};