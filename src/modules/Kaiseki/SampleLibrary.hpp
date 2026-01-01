#pragma once

#include "Pack.hpp"

class SampleLibrary {
private:
    std::vector<Pack> packs;
    
public:
    SampleLibrary() {}
    
    // Load all packs from JSON file
    bool loadFromJson(const std::string& json_path) {
        FILE* file = fopen(json_path.c_str(), "r");
        if (!file) return false;
        
        json_error_t error;
        json_t* root_json = json_loadf(file, 0, &error);
        fclose(file);
        
        if (!root_json) return false;
        
        // Get packs array
        json_t* packs_json = json_object_get(root_json, "packs");
        if (!packs_json || !json_is_array(packs_json)) {
            json_decref(root_json);
            return false;
        }
        
        // Get pack directory (same directory as JSON file)
        std::string pack_dir = rack::system::getDirectory(json_path);
        
        // Clear existing packs
        packs.clear();
        
        // Load each pack
        size_t num_packs = json_array_size(packs_json);
        for (size_t i = 0; i < num_packs; i++) {
            json_t* pack_json = json_array_get(packs_json, i);
            if (pack_json) {
                Pack pack;
                if (pack.loadFromJson(pack_json, pack_dir)) {
                    packs.push_back(pack);
                } else {
                    // Add empty pack to maintain index consistency
                    Pack empty_pack;
                    packs.push_back(empty_pack);
                }
            }
        }
        
        json_decref(root_json);
        return true;
    }
    
    // Access methods
    size_t size() {
        return packs.size();
    }
    
    bool empty() {
        return packs.empty();
    }
    
    Pack* getPack(size_t index) {
        if (index < packs.size()) {
            return &packs[index];
        }
        return nullptr;
    }
    
    // Vector-like access
    Pack& operator[](size_t index) {
        return packs[index];
    }
    
    // Iterator support
    std::vector<Pack>::iterator begin() {
        return packs.begin();
    }
    
    std::vector<Pack>::iterator end() {
        return packs.end();
    }
    
    // Utility methods
    void clear() {
        packs.clear();
    }
    
    // Add individual pack
    void addPack(const Pack& pack) {
        packs.push_back(pack);
    }
    
    // Remove pack
    void removePack(size_t index) {
        if (index < packs.size()) {
            packs.erase(packs.begin() + index);
        }
    }
    
    // Get a specific track from pack/track indices
    Track* getTrack(size_t pack_index, size_t track_index) {
        Pack* pack = getPack(pack_index);
        if (pack) {
            return pack->getTrack(track_index);
        }
        return nullptr;
    }
    
    // Get pack names for display
    std::string getPackName(size_t index) {
        Pack* pack = getPack(index);
        if (pack) {
            return pack->getName();
        }
        return "";
    }
};