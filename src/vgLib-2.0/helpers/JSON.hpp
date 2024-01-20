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
        static void deserializeSequencer(json_t *json_sequencer, VoltageSequencer &sequencer)
        {
            // Ensure the JSON object exists and is correctly formatted, if not return
            if (!json_sequencer || !json_is_object(json_sequencer))
                return;

            // Load sequence values
            json_t *sequencer_json = json_object_get(json_sequencer, "sequence");

            // Backwards compatibility with old format
            if(!sequencer_json || !json_is_array(sequencer_json))
            {
                sequencer_json = json_object_get(json_sequencer, "values");
            }

            if (sequencer_json && json_is_array(sequencer_json))
            {
                size_t index;
                json_t *value_json;
                json_array_foreach(sequencer_json, index, value_json)
                {
                    if (index < (unsigned int) sequencer.getMaxLength())
                    {
                        sequencer.setValue(index, json_real_value(value_json));
                    }
                }
            }

            // Load window_start
            json_t *window_start_json = json_object_get(json_sequencer, "window_start");
            if (window_start_json && json_is_integer(window_start_json))
            {
                sequencer.setWindowStart(json_integer_value(window_start_json));
            }

            // Load window_end
            json_t *window_end_json = json_object_get(json_sequencer, "window_end");
            if (window_end_json && json_is_integer(window_end_json))
            {
                sequencer.setWindowEnd(json_integer_value(window_end_json));
            }
        }


        static json_t *serializeSequencer(VoltageSequencer &sequencer)
        {
            json_t *sequencer_data_json = json_object();

            // Create a JSON array for the sequence values
            json_t *sequence_array = json_array();
            for (unsigned int column = 0; column < (unsigned int) sequencer.getMaxLength(); column++)
            {
                json_array_append_new(sequence_array, json_real(sequencer.getValue(column)));
            }

            // Add sequence array and other properties to the JSON object
            json_object_set_new(sequencer_data_json, "sequence", sequence_array);
            json_object_set_new(sequencer_data_json, "window_start", json_integer(sequencer.getWindowStart()));
            json_object_set_new(sequencer_data_json, "window_end", json_integer(sequencer.getWindowEnd()));

            return sequencer_data_json;
        }


        static json_t *serializeVoltageSequencer(VoltageSequencer &sequencer)
        {
            json_t *sequencer_json = saveSequencer(sequencer);

            // Add specific attributes for VoltageSequencer
            json_object_set_new(sequencer_json, "polarity", json_integer(sequencer.getPolarity()));
            json_object_set_new(sequencer_json, "snap_division", json_integer(sequencer.snap_division));
            json_object_set_new(sequencer_json, "range_low", json_real(sequencer.range_low));
            json_object_set_new(sequencer_json, "range_high", json_real(sequencer.range_high));
            json_object_set_new(sequencer_json, "sample_and_hold", json_boolean(sequencer.sample_and_hold));

            return sequencer_json;
        }


        static json_t *serializeVoltageSequencers(const std::map<std::string, VoltageSequencer>& sequencers)
        {
            json_t *all_sequencers_json = json_object();

            for (const auto& [key, sequencer] : sequencers)
            {
                json_t *sequencer_json = serializeVoltageSequencer(sequencer);
                json_object_set_new(all_sequencers_json, key.c_str(), sequencer_json);
            }

            return all_sequencers_json;
        }

        // deserializeVoltageSequencer
        // This function is used to load a VoltageSequencer from a JSON object

        static void deserializeVoltageSequencer(json_t *json_sequencer, VoltageSequencer &sequencer)
        {
            if (!json_sequencer || !json_is_object(json_sequencer))
            {
                return;
            }

            // Directly load the sequencer data
            loadSequencer(json_sequencer, sequencer);

            // Load specific attributes for VoltageSequencer
            sequencer.setPolarity(static_cast<VoltageSequencer::Polarity>(getInteger(json_sequencer, "polarity")));
            sequencer.snap_division = getInteger(json_sequencer, "snap_division");
            sequencer.range_low = getNumber(json_sequencer, "range_low");
            sequencer.range_high = getNumber(json_sequencer, "range_high");
            sequencer.sample_and_hold = getBoolean(json_sequencer, "sample_and_hold");
        }
        */
    };

} // namespace vgLib_v2