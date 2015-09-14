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

#include "Fav_Tracks_Handler.h"

FavTracksMemoryModel::FavTracksMemoryModel(char* filename): trackFilename(filename) {
	favTracksFile.open(filename, std::fstream::in | std::fstream::out | std::fstream::app);
	//some is_open logic exception

	if (favTracksFile.is_open()) {
		std::wstring contents;
		favTracksFile.seekg(0, std::ios::end);
		contents.resize(favTracksFile.tellg());
		favTracksFile.seekg(0, std::ios::beg);
		favTracksFile.read(&contents[0], contents.size());
		std::wistringstream iss(contents);
		std::wstring wstrsong;
		size_t file_lines_count = std::count(std::istreambuf_iterator<wchar_t>(iss), std::istreambuf_iterator<wchar_t>(), L'\n');
		iss.clear();
		iss.seekg(0, std::ios::beg);
		favTracksVector.reserve(file_lines_count);
		size_t track_begin_idx = 0;
		bool fileRewriteFlag = false;
		while (!iss.eof()) {
			std::getline(iss, wstrsong);
			if (!wstrsong.length() || wstrsong[0] == L'\0')
				continue; //just skip empty lines and go on
			size_t hash = GetHash(wstrsong);
			if (!favTracksMap.count(hash)) {
				favTracksVector.push_back(wstrsong);
				favTracksMap[hash] = std::make_pair(track_begin_idx, favTracksVector.size() - 1); //wiring map to vector, track_end_idx calculated in C++11 O(1), so not needed
				track_begin_idx += wstrsong.length();
			}
			else
				fileRewriteFlag = true;
		}
		if (fileRewriteFlag) {
			favTracksFile.close();
			std::remove(trackFilename.c_str()); //deletes prev file
			favTracksFile.open(trackFilename.c_str(), std::fstream::in | std::fstream::out | std::fstream::app);
			for (size_t vecIdx = 0; vecIdx < favTracksVector.size(); ++vecIdx)
				favTracksFile << favTracksVector[vecIdx] << L'\n';
		}
	}
	//if ((favTracksFile.rdstate() & std::ifstream::failbit) != 0 && (favTracksFile.rdstate() & std::ifstream::eofbit) != 0) //--> and here
	//if ((favTracksFile.rdstate() & std::ifstream::badbit) != 0)
	//if ((favTracksFile.rdstate() & std::ifstream::goodbit) != 0)
	favTracksFile.clear(); //reset rdstate for further io-write
	//favTracksFile.seekg(std::ios_base::end);
	numberOfRecordedTracks = 0;
};

void FavTracksMemoryModel::AddTrackToModel(const std::wstring& trackName, const std::wstring& artistName) {
	if (!IsTrackInModel(trackName, artistName)) {
		std::wstring track = artistName + trackName;
		size_t trackHash = GetHash(track);
		size_t newTrackPosition = 0;
		if (favTracksVector.size()) {
			std::wstring prev_track = favTracksVector[favTracksVector.size() - 1];
			size_t prev_track_hash = GetHash(prev_track);
			newTrackPosition = favTracksMap[prev_track_hash].first + track.length();
		}
		favTracksVector.push_back(track);
		favTracksMap[trackHash] = std::make_pair(newTrackPosition, favTracksVector.size() - 1);
		if (favTracksFile.is_open()) {
			favTracksFile << artistName << L" - " << trackName << std::endl;
			//favTracksFile.flush();
		}
	}
};

size_t FavTracksMemoryModel::GetHash(std::wstring song) const {
	std::wstring uniwsong;
	unifiedWstringfromWstring(uniwsong, song, song.begin(), song.end());
	return trackHasher(uniwsong);
};

bool FavTracksMemoryModel::IsTrackInModel(const std::wstring& trackName, const std::wstring& artistName) const {
	if (favTracksMap.count(GetHash(artistName + trackName)))
		return true;
	return false;
};

void FavTracksMemoryModel::RemoveTrack(const std::wstring& trackName, const std::wstring& artistName) {
	std::wstring track = artistName + trackName;
	size_t songHash = GetHash(track);
	size_t songLn = track.length();
	size_t idx = favTracksMap[songHash].second;
	favTracksVector.erase(favTracksVector.begin() + idx);

	for (idx; idx < favTracksVector.size(); ++idx) {
		size_t tempSongHash = GetHash(favTracksVector[idx]);
		favTracksMap[tempSongHash].first -= songLn; //in file track has decremented by /removed track length/ position
		--favTracksMap[tempSongHash].second; //idx in vector decremented, so did in map
	}

	favTracksFile.close();
	std::remove(trackFilename.c_str()); //deletes prev file
	favTracksFile.open(trackFilename.c_str(), std::fstream::in | std::fstream::out | std::fstream::app);
	for (size_t vecIdx = 0; vecIdx < favTracksVector.size(); ++vecIdx)
		favTracksFile << favTracksVector[vecIdx] << L'\n';

	numberOfRecordedTracks++; // from now track removed only if captured
};

size_t FavTracksMemoryModel::GetNumberTracksInPlaylist() const {
	return favTracksMap.size();
};

size_t FavTracksMemoryModel::TracksRecorded() const {
	return numberOfRecordedTracks;
};

FavTracksMemoryModel::~FavTracksMemoryModel() {
	if (favTracksFile.is_open()) {
		favTracksFile.close();
	}
};