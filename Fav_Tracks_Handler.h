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

#ifndef FAV_TRACKS_HANDLER_H_
#define FAV_TRACKS_HANDLER_H_

#include "utility.h"

#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>

//Stores hashes, file offsets, names of all songs for recording. Works with file of tracks.

class FavTracksMemoryModel {
private:
	std::unordered_map<size_t, std::pair<std::size_t, std::size_t>> favTracksMap; //Map of: key - song hash, value - pair<start position (offset) in file, favTracksVector index of song>
	std::vector<std::wstring> favTracksVector; //Vector of songs, order in vector is order in output file
	std::wfstream favTracksFile;
	std::hash<std::wstring> trackHasher;
	std::string trackFilename;
	size_t numberOfRecordedTracks;
	size_t GetHash(std::wstring song) const;

public:
	void AddTrackToModel(const std::wstring& trackName, const std::wstring& artistName);
	bool IsTrackInModel(const std::wstring& trackName, const std::wstring& artistName) const;
	void RemoveTrack(const std::wstring& trackName, const std::wstring& artistName);
	FavTracksMemoryModel(char* filename);
	~FavTracksMemoryModel();
	size_t GetNumberTracksInPlaylist() const;
	size_t TracksRecorded() const;
};

#endif