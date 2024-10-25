// TODO:
// 3. Click on bottom waveform to select upper position
// 4. Explore caching of waveform renderings
// 5. Fix autobreak studio
// 6. Render markers on bottom waveform
// 7. Rename module
// 8. Menu item for setting trigger lengths
// 9. Context menu for marker: Set to specific sample location
// 10. Menu item for automatically setting markers??
// 11. Improve zoom experience

#include "Marker.hpp"

struct FourTrack : VoxglitchSamplerModule
{
    std::string loaded_filename = "[ EMPTY ]";

    TrackModel track;
    Sample sample;
    WaveformModel waveform_model;

    dsp::SchmittTrigger start_trigger;
    dsp::SchmittTrigger stop_trigger;
    dsp::SchmittTrigger reset_trigger;

    // Add these new members
    bool playing = false;
    unsigned int playback_position = 0;
    float output_left = 0.0f;
    float output_right = 0.0f;
    int active_marker = 0; // Track which marker output is currently selected (0-31)
    dsp::PulseGenerator marker_pulse_generators[32];
    dsp::PulseGenerator reset_light_pulse;
    std::map<unsigned int, std::vector<Marker>> markers; // position -> markers at that position

    enum ParamIds
    {
        ENUMS(MARKER_BUTTONS, 32),
        START_BUTTON,
        STOP_BUTTON,
        RESET_BUTTON,
        NUM_PARAMS
    };
    enum InputIds
    {
        START_INPUT,
        STOP_INPUT,
        RESET_INPUT,
        NUM_INPUTS
    };
    enum OutputIds
    {
        OUTPUT_LEFT,
        OUTPUT_RIGHT,
        ENUMS(MARKER_OUTPUTS, 32),
        NUM_OUTPUTS
    };
    enum LightIds
    {
        ENUMS(MARKER_LIGHTS, 32),
        START_LIGHT,
        STOP_LIGHT,
        RESET_LIGHT,
        NUM_LIGHTS
    };

    FourTrack()
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        // sample.load("e:/dev/example.wav");
        track.setSample(&sample);

        // Set up the callback
        track.onMarkerSelected = [this](int output_number)
        {
            params[MARKER_BUTTONS + active_marker].setValue(0.f);
            params[MARKER_BUTTONS + output_number].setValue(1.f);
            active_marker = output_number;
        };

        // Set up the callback for synchronizing markers with the waveform
        track.onSyncMarkers = [this]()
        {
            syncMarkers();
        };

        // Set the waveform model sample for the lower waveform display
        waveform_model.sample = &sample;
        waveform_model.visible = true;

        // Configure parameters and their corresponding lights
        for (int i = 0; i < 32; i++)
        {
            configParam(MARKER_BUTTONS + i, 0.f, 1.f, 0.f, "Marker Selection Button #" + std::to_string(i));
            configLight(MARKER_LIGHTS + i, "Select #" + std::to_string(i));
        }

        for (int i = 0; i < 32; i++)
        {
            configParam(MARKER_BUTTONS + i, 0.f, 1.f, 0.f, "Marker Selection Button #" + std::to_string(i));
        }
        params[MARKER_BUTTONS].setValue(1.f);
        active_marker = 0;

        // Configure the transport buttons
        configSwitch(START_BUTTON, 0.f, 1.f, 0.f, "Start");
        configSwitch(STOP_BUTTON, 0.f, 1.f, 0.f, "Stop");
        configSwitch(RESET_BUTTON, 0.f, 1.f, 0.f, "Reset");

        track.setMarkers(&markers);
    }

    /*
    void addMarker(unsigned int position)
    {
        DEBUG("FourTrack:: Adding marker at position %d", position);
        markers[position].push_back(Marker(active_marker));
        synchronizeMarkersWithWaveform();
    }


    void removeMarker(unsigned int position)
    {
        markers.erase(position);
        synchronizeMarkersWithWaveform();
    }
    */

    void clearMarkers()
    {
        markers.clear();
        syncMarkers();
    }


    // Optionally, remove markers for a specific output at a position
    /*
    void removeMarkerForOutput(unsigned int position, int output_number)
    {
        auto it = markers.find(position);
        if (it != markers.end())
        {
            auto &marker_list = it->second;
            marker_list.erase(
                std::remove_if(
                    marker_list.begin(),
                    marker_list.end(),
                    [output_number](const Marker &m)
                    { return m.output_number == output_number; }),
                marker_list.end());
            // If no markers left at this position, remove the map entry
            if (marker_list.empty())
            {
                markers.erase(it);
            }
        }
    }
    */

    void syncMarkers()
    {
        waveform_model.marker_positions.clear();
        for (const auto &marker_pair : markers)
        {
            // Assuming each position in the map represents a sample position for a marker
            waveform_model.marker_positions.push_back(marker_pair.first);
        }
    }

    // Autosave module data.  VCV Rack decides when this should be called.

    json_t *dataToJson() override
    {
        json_t *rootJ = json_object();

        json_object_set_new(rootJ, "loaded_sample_path", json_string(sample.getPath().c_str()));

        json_t *markersJ = json_array();
        for (const auto &pos_markers : markers)
        {
            for (const Marker &m : pos_markers.second)
            {
                json_t *markerJ = json_object();
                json_object_set_new(markerJ, "position", json_integer(pos_markers.first));
                json_object_set_new(markerJ, "output", json_integer(m.output_number));
                json_array_append_new(markersJ, markerJ);
            }
        }
        json_object_set_new(rootJ, "markers", markersJ);

        // Call VoxglitchSamplerModule::saveSamplerData to save sampler data
        saveSamplerData(rootJ);

        return rootJ;
    }

    void dataFromJson(json_t *rootJ) override
    {

        // Load the sample
        json_t *loaded_sample_path = json_object_get(rootJ, ("loaded_sample_path"));
        if (loaded_sample_path)
        {
            sample.load(json_string_value(loaded_sample_path));
            loaded_filename = sample.getFilename();
        }

        // Load the markers
        markers.clear();
        json_t *markersJ = json_object_get(rootJ, "markers");
        if (markersJ)
        {
            size_t i;
            json_t *markerJ;
            json_array_foreach(markersJ, i, markerJ)
            {
                unsigned int pos = json_integer_value(json_object_get(markerJ, "position"));
                int out = json_integer_value(json_object_get(markerJ, "output"));
                markers[pos].push_back(Marker(out));
            }
        }
        syncMarkers();


        // Call VoxglitchSamplerModule::loadSamplerData to load sampler specific data
        loadSamplerData(rootJ);
    }

    void process(const ProcessArgs &args) override
    {
        bool reset_triggered = reset_trigger.process(inputs[RESET_INPUT].getVoltage(), constants::gate_low_trigger, constants::gate_high_trigger);
        bool start_triggered = start_trigger.process(inputs[START_INPUT].getVoltage(), constants::gate_low_trigger, constants::gate_high_trigger);
        bool stop_triggered = stop_trigger.process(inputs[STOP_INPUT].getVoltage(), constants::gate_low_trigger, constants::gate_high_trigger);

        // Also handle button presses
        if (params[START_BUTTON].getValue() > 0.f)
        {
            start_triggered = true;
            stop_triggered = false;
            params[START_BUTTON].setValue(0.f);
        }

        if (params[STOP_BUTTON].getValue() > 0.f)
        {
            stop_triggered = true;
            start_triggered = false;
            params[STOP_BUTTON].setValue(0.f);
        }

        if (params[RESET_BUTTON].getValue() > 0.f)
        {
            reset_triggered = true;
            params[RESET_BUTTON].setValue(0.f);
        }

        if (start_triggered)
        {
            playing = true;
        }

        if (stop_triggered)
        {
            playing = false;
        }

        if (reset_triggered)
        {
            reset_light_pulse.trigger(0.15);
            playback_position = 0;
        }

        // Handle playback
        if (playing && sample.loaded)
        {
            auto it = markers.find(playback_position);
            if (it != markers.end())
            {
                // Trigger pulses for all markers at this position
                for (const Marker &marker : it->second)
                {
                    marker_pulse_generators[marker.output_number].trigger(1e-3f); // 1ms trigger
                }
            }

            // Process all pulse generators and set outputs
            for (int i = 0; i < 32; i++)
            {
                outputs[MARKER_OUTPUTS + i].setVoltage(
                    marker_pulse_generators[i].process(args.sampleTime) ? 10.0f : 0.0f);
            }

            if (playback_position < sample.size())
            {
                sample.read(playback_position, &output_left, &output_right);
                playback_position++;

                // Update waveform position indicator
                waveform_model.playback_percentage = float(playback_position) / float(sample.size());
            }
            else
            {
                // Reached end of sample
                playing = false;
                playback_position = 0;
                waveform_model.playback_percentage = 0.0f;
            }
        }

        outputs[OUTPUT_LEFT].setVoltage(output_left * 5.0f); // Scale by 5V for typical modular levels
        outputs[OUTPUT_RIGHT].setVoltage(output_right * 5.0f);

        // Handle marker button changes
        for (int i = 0; i < 32; i++)
        {
            // If this button was just pressed
            if (params[MARKER_BUTTONS + i].getValue() > 0.f && i != active_marker)
            {
                // Clear old active button
                params[MARKER_BUTTONS + active_marker].setValue(0.f);
                // Set new active button
                active_marker = i;
                params[MARKER_BUTTONS + i].setValue(1.f);

                track.setActiveMarker(active_marker);
            }
            // Ensure active marker stays pressed
            else if (i == active_marker && params[MARKER_BUTTONS + i].getValue() == 0.f)
            {
                params[MARKER_BUTTONS + i].setValue(1.f);
            }
            // Update lights to match button states
            lights[MARKER_LIGHTS + i].setBrightness(params[MARKER_BUTTONS + i].getValue());
        }

        // light up start, stop, reset buttons
        lights[START_LIGHT].setBrightness(playing ? 1.f : 0.f);
        lights[STOP_LIGHT].setBrightness(playing ? 0.f : 1.f);
        lights[RESET_LIGHT].setBrightness(reset_light_pulse.process(args.sampleTime) ? 1.f : 0.f);
    }
};
