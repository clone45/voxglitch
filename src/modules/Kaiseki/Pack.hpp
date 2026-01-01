#pragma once

#include "Track.hpp"

class Pack {
private:
    std::vector<Track> tracks;
    std::string pack_name;
    
public:
    Pack() : pack_name("") {}
    
    Pack(const std::string& name) : pack_name(name) {}
    
    // Load tracks from JSON pack data
    bool loadFromJson(json_t* pack_json, const std::string& pack_dir) {
        if (!pack_json || !json_is_object(pack_json)) {
            return false;
        }
        
        // Get pack name
        json_t* name_json = json_object_get(pack_json, "name");
        if (name_json && json_is_string(name_json)) {
            pack_name = json_string_value(name_json);
        }
        
        // Get pack-level folder (optional)
        std::string pack_folder;
        json_t* folder_json = json_object_get(pack_json, "folder");
        if (folder_json && json_is_string(folder_json)) {
            pack_folder = json_string_value(folder_json);
        }
        
        // Get tracks array
        json_t* tracks_json = json_object_get(pack_json, "tracks");
        if (!tracks_json || !json_is_array(tracks_json)) {
            return false;
        }
        
        // Clear existing tracks
        tracks.clear();
        
        // Load each track
        size_t num_tracks = json_array_size(tracks_json);
        for (size_t i = 0; i < num_tracks; i++) {
            json_t* track_json = json_array_get(tracks_json, i);
            if (track_json) {
                Track track;
                if (track.loadFromJson(track_json, pack_dir, pack_folder)) {
                    tracks.push_back(track);
                } else {
                    // Add empty track to maintain index consistency
                    Track empty_track;
                    tracks.push_back(empty_track);
                }
            }
        }
        
        return true;
    }
    
    // Access methods
    size_t size() {
        return tracks.size();
    }
    
    bool empty() {
        return tracks.empty();
    }
    
    Track* getTrack(size_t index) {
        if (index < tracks.size()) {
            return &tracks[index];
        }
        return nullptr;
    }
    
    // Vector-like access
    Track& operator[](size_t index) {
        return tracks[index];
    }
    
    // Iterator support
    std::vector<Track>::iterator begin() {
        return tracks.begin();
    }
    
    std::vector<Track>::iterator end() {
        return tracks.end();
    }
    
    // Utility methods
    std::string& getName() {
        return pack_name;
    }
    
    void setName(const std::string& name) {
        pack_name = name;
    }
    
    void clear() {
        tracks.clear();
        pack_name = "";
    }
    
    // Add individual track
    void addTrack(const Track& track) {
        tracks.push_back(track);
    }
    
    // Remove track
    void removeTrack(size_t index) {
        if (index < tracks.size()) {
            tracks.erase(tracks.begin() + index);
        }
    }
};