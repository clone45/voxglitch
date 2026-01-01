#pragma once

#include "KaisekiSamplePlayer.hpp"

// Track class for Kaiseki - adapted from Tracks module
class Track {
public:
    KaisekiSamplePlayer sample_player;
    std::string name;
    float bpm;
    std::vector<unsigned int> cue_points;
    int repeat_count = -1;  // -1 = infinite, 0 = one-shot, N = finite loops
    
    Track() : bpm(120.0f) {
        name = "[ EMPTY ]";
        cue_points.clear();
    }
    
    Track(const std::string& track_name, float track_bpm) 
        : name(track_name), bpm(track_bpm) {
        cue_points.clear();
    }
    
    // Load sample and metadata from JSON track data  
    bool loadFromJson(json_t* track_json, const std::string& pack_dir) {
        return loadFromJson(track_json, pack_dir, "");
    }
    
    // Load sample and metadata from JSON track data with optional folder override
    bool loadFromJson(json_t* track_json, const std::string& pack_dir, const std::string& default_folder = "") {
        if (!track_json || !json_is_object(track_json)) {
            return false;
        }
        
        // Get filename
        json_t* filename_json = json_object_get(track_json, "filename");
        json_t* name_json = json_object_get(track_json, "name");
        json_t* bpm_json = json_object_get(track_json, "bpm");
        json_t* cue_points_json = json_object_get(track_json, "cue_points");
        json_t* repeat_json = json_object_get(track_json, "repeat");
        json_t* folder_json = json_object_get(track_json, "folder");
        
        // Load sample file
        if (filename_json && json_is_string(filename_json)) {
            std::string filename = json_string_value(filename_json);
            if (!filename.empty()) {
                // Determine folder to use
                std::string folder;
                if (folder_json && json_is_string(folder_json)) {
                    // Use folder from JSON
                    folder = json_string_value(folder_json);
                } else if (!default_folder.empty()) {
                    // Use provided default folder
                    folder = default_folder;
                } else {
                    // Fallback: derive from pack directory name
                    folder = "pack1"; // Legacy default
                }
                
                std::string full_path = pack_dir + "/" + folder + "/" + filename;
                if (sample_player.loadSample(full_path)) {
                    // Set name
                    if (name_json && json_is_string(name_json)) {
                        name = json_string_value(name_json);
                    } else {
                        name = sample_player.getFilename();
                    }
                } else {
                    name = "[ EMPTY ]";
                    return false;
                }
            } else {
                name = "[ EMPTY ]";
                return false;
            }
        } else {
            name = "[ EMPTY ]";
            return false;
        }
        
        // Get BPM
        if (bpm_json && json_is_number(bpm_json)) {
            bpm = json_number_value(bpm_json);
        } else {
            bpm = 120.0f;
        }
        
        // Get cue points
        cue_points.clear();
        if (cue_points_json && json_is_array(cue_points_json)) {
            size_t num_cues = json_array_size(cue_points_json);
            for (size_t j = 0; j < num_cues; j++) {
                json_t* cue_json = json_array_get(cue_points_json, j);
                if (cue_json && json_is_number(cue_json)) {
                    unsigned int cue_sample = (unsigned int)json_integer_value(cue_json);
                    cue_points.push_back(cue_sample);
                }
            }
        }
        
        // Get repeat count (default to infinite loop for backward compatibility)
        repeat_count = -1;
        if (repeat_json && json_is_number(repeat_json)) {
            repeat_count = (int)json_integer_value(repeat_json);
        }
        
        // Configure the sample player with repeat settings
        sample_player.setRepeatCount(repeat_count);
        
        return true;
    }
    
    // Convenience methods
    bool isLoaded() {
        return sample_player.isLoaded();
    }
    
    std::string& getName() {
        return name;
    }
    
    float getBPM() {
        return bpm;
    }
    
    std::vector<unsigned int>& getCuePoints() {
        return cue_points;
    }
    
    KaisekiSamplePlayer& getSamplePlayer() {
        return sample_player;
    }
    
    int getRepeatCount() {
        return repeat_count;
    }
    
    void setRepeatCount(int repeat) {
        repeat_count = repeat;
        sample_player.setRepeatCount(repeat);
    }
};