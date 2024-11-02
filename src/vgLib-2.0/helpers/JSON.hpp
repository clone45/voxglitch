#pragma once

#include <jansson.h>
#include <stdbool.h>

namespace vgLib_v2
{

    class JSON
    {

    public:
        static double getNumber(json_t *root, const char *key)
        {
            json_t *json_value = json_object_get(root, key);
            if (json_value && json_is_number(json_value))
            {
                return json_number_value(json_value);
            }
            return 0.0; // or another default value if required
        }

        static int getInteger(json_t *root, const char *key)
        {
            json_t *json_value = json_object_get(root, key);
            if (json_value && json_is_integer(json_value))
            {
                return json_integer_value(json_value);
            }
            return 0; // or another default value if required
        }

        static bool getBoolean(json_t *root, const char *key)
        {
            json_t *json_value = json_object_get(root, key);
            if (json_value && json_is_boolean(json_value))
            {
                return json_boolean_value(json_value);
            }
            return false; // or another default value if required
        }

        /*

        // TODO: These should probably be moved into a separate helper class for specifically loading and saving sequencers
        // Case in point: NoteReadout module uses the JSON.hpp file for the getNumber, getInteger, and getBoolean functions,
        // but does not use the loadSequencer and saveSequencer functions.  And needing the VoltageSequencer class to be
        // included for the JSON.hpp file to work is not ideal.

        static void loadSequencer(json_t *json, const std::string &sequencer_name, VoltageSequencer &sequencer)
        {
            json_t *sequencer_data_json = json_object_get(json, sequencer_name.c_str());

            // Ensure the JSON object exists and is correctly formatted, if not return
            if (!sequencer_data_json || !json_is_object(sequencer_data_json))
                return;

            // Load sequence values
            json_t *sequencer_json = json_object_get(sequencer_data_json, "sequence");
            if (sequencer_json && json_is_array(sequencer_json))
            {
                size_t index;
                json_t *value_json;
                json_array_foreach(sequencer_json, index, value_json)
                {
                    if (index < NUMBER_OF_STEPS)
                    {
                        sequencer.setValue(index, json_real_value(value_json));
                    }
                }
            }

            // Load window_start
            json_t *window_start_json = json_object_get(sequencer_data_json, "window_start");
            if (window_start_json && json_is_integer(window_start_json))
            {
                sequencer.setWindowStart(json_integer_value(window_start_json));
            }

            // Load window_end
            json_t *window_end_json = json_object_get(sequencer_data_json, "window_end");
            if (window_end_json && json_is_integer(window_end_json))
            {
                sequencer.setWindowEnd(json_integer_value(window_end_json));
            }
        }

        static json_t *saveSequencer(VoltageSequencer &sequencer)
        {
            json_t *sequencer_json = json_array();
            for (unsigned int column = 0; column < NUMBER_OF_STEPS; column++)
            {
                json_array_append_new(sequencer_json, json_real(sequencer.getValue(column)));
            }

            // Wrap the array and other properties in an object
            json_t *sequencer_data_json = json_object();
            json_object_set_new(sequencer_data_json, "sequence", sequencer_json);
            json_object_set_new(sequencer_data_json, "window_start", json_integer(sequencer.getWindowStart()));
            json_object_set_new(sequencer_data_json, "window_end", json_integer(sequencer.getWindowEnd()));

            return sequencer_data_json;
        }
        */
    };

} // namespace vgLib_v2