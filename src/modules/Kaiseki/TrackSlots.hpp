#pragma once

#include "Track.hpp"

class TrackSlots {
private:
    static const int NUM_SLOTS = 8;
    Track* slots[NUM_SLOTS];
    
public:
    TrackSlots() {
        // Initialize all slots to null
        for (int i = 0; i < NUM_SLOTS; i++) {
            slots[i] = nullptr;
        }
    }
    
    // Assign a track to a specific slot
    void assignTrack(int slot_index, Track* track) {
        if (slot_index >= 0 && slot_index < NUM_SLOTS) {
            slots[slot_index] = track;
        }
    }
    
    // Get the track for a specific slot
    Track* getTrack(int slot_index) {
        if (slot_index >= 0 && slot_index < NUM_SLOTS) {
            return slots[slot_index];
        }
        return nullptr;
    }
    
    // Clear a specific slot
    void clearSlot(int slot_index) {
        if (slot_index >= 0 && slot_index < NUM_SLOTS) {
            slots[slot_index] = nullptr;
        }
    }
    
    // Clear all slots
    void clearAllSlots() {
        for (int i = 0; i < NUM_SLOTS; i++) {
            slots[i] = nullptr;
        }
    }
    
    // Check if a slot has a track assigned
    bool hasTrack(int slot_index) {
        Track* track = getTrack(slot_index);
        return track != nullptr;
    }
    
    // Check if a slot has a loaded track
    bool isSlotLoaded(int slot_index) {
        Track* track = getTrack(slot_index);
        return track != nullptr && track->isLoaded();
    }
    
    // Get track name for display (safe)
    std::string getTrackName(int slot_index) {
        Track* track = getTrack(slot_index);
        if (track) {
            return track->getName();
        }
        return "[ EMPTY ]";
    }
    
    // Get track BPM (safe)
    float getTrackBPM(int slot_index) {
        Track* track = getTrack(slot_index);
        if (track) {
            return track->getBPM();
        }
        return 120.0f;
    }
    
    // Get cue points for a slot (safe)
    std::vector<unsigned int> getTrackCuePoints(int slot_index) {
        Track* track = getTrack(slot_index);
        if (track) {
            return track->getCuePoints();
        }
        return std::vector<unsigned int>();
    }
    
    // Get SamplePlayer for a slot (safe)
    KaisekiSamplePlayer* getSamplePlayer(int slot_index) {
        Track* track = getTrack(slot_index);
        if (track) {
            return &track->getSamplePlayer();
        }
        return nullptr;
    }
    
    // Get number of slots (constant)
    int getNumSlots() {
        return NUM_SLOTS;
    }
};