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

#include <map>
#include <string>
#include <utility>

std::pair<std::string, std::string> getArtistAndTrackName(std::string& song);

void unifiedWstringfromString(std::wstring& uniWstr, const std::string& sourceStr, size_t begin, size_t end);
void unifiedWstringfromWstring(std::wstring& uniWstr, const std::wstring& sourceWStr, size_t begin, size_t end);
void unifiedWstringfromWstring(std::wstring& uniWstr, const std::wstring& sourceWStr, std::wstring::iterator begin, std::wstring::iterator end);
//get hash from concat of artist name and trackname 
size_t getSongHash(std::string& song);
size_t getSongHash(wchar_t* song_chars);