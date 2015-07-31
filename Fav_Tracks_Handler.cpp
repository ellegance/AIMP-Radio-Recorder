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

FavTracksMemoryModel::FavTracksMemoryModel(char* filename){
	favTracksFile.open(filename, std::fstream::in | std::fstream::out | std::fstream::app);
	//some is_open logic exception
	if (favTracksFile.is_open()){
		wchar_t song[512]; //
		while (favTracksFile.getline(song, 512, L'\n')){
			//favTracks.insert(getSongHash(song));
			std::wstring wstrsong(song);
			size_t hash = getHash(wstrsong);
			favTracks.insert(hash);
		}
	} //here you mb reached wrong position and file indicator fails for io operations

	//if ((favTracksFile.rdstate() & std::ifstream::failbit) != 0 && (favTracksFile.rdstate() & std::ifstream::eofbit) != 0) //--> and here
	//if ((favTracksFile.rdstate() & std::ifstream::badbit) != 0)
	//if ((favTracksFile.rdstate() & std::ifstream::goodbit) != 0)
	favTracksFile.clear(); //reset rdstate for further io-write
	//favTracksFile.seekg(std::ios_base::end);
	numberOfRecordedTracks = 0;
};

void FavTracksMemoryModel::AddTrackToModel(const std::wstring& trackName, const std::wstring& artistName){
	if (!IsTrackInModel(trackName, artistName)){
		favTracks.insert(getHash(artistName + trackName));
		if (favTracksFile.is_open()){
			favTracksFile << artistName << L" - " << trackName << std::endl;
			//favTracksFile.flush();
		}
	}
};

std::wstring FavTracksMemoryModel::GetAllHashes() const{
	std::wstring allHashes;
	for (auto hash : favTracks)
		allHashes += std::to_wstring(hash) + L' ';
	return allHashes;
};

size_t FavTracksMemoryModel::getHash(std::wstring song) const{
	std::wstring uniwsong;
	unifiedWstringfromWstring(uniwsong, song, song.begin(), song.end());
	return trackHasher(uniwsong);
};

bool FavTracksMemoryModel::IsTrackInModel(const std::wstring& trackName, const std::wstring& artistName) const {
	if (favTracks.count(getHash(artistName + trackName)))
		return true;
	return false;
};

void FavTracksMemoryModel::RemoveTrackFromModel(const std::wstring& trackName, const std::wstring& artistName){
	favTracks.erase(getHash(artistName + trackName));
	numberOfRecordedTracks++;
};

size_t FavTracksMemoryModel::GetNumberTracksInPlaylist() const {
	return favTracks.size();
};

size_t FavTracksMemoryModel::TracksRecorded() const{
	return numberOfRecordedTracks;
};

FavTracksMemoryModel::~FavTracksMemoryModel(){
	if (favTracksFile.is_open()){
		favTracksFile.close();
	}
};