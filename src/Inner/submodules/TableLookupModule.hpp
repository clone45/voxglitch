#pragma once
#include "../BaseModule.hpp"
#include "../dsp/SchmittTrigger.hpp"

class TableLookupModule : public BaseModule {
public:
    enum INPUTS {
        IN,
        TABLE,
        NUM_INPUTS
    };

    enum OUTPUTS {
        OUTPUT,
        NUM_OUTPUTS
    };

    enum PARAMS {
        NUM_PARAMS
    };

    TableLookupModule(json_t* data) 
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS);

        json_t* tables = json_object_get(data, "tables");
        if (tables && json_is_array(tables)) 
        {
            size_t index;
            json_t* table;
            json_array_foreach(tables, index, table) 
            {
                if (json_is_array(table)) 
                {
                    std::vector<float> table_values;
                    size_t table_index;
                    json_t* table_value;
                    json_array_foreach(table, table_index, table_value) 
                    {
                        table_values.push_back(json_number_value(table_value));
                    }
                    this->tables.push_back(table_values);
                }
                else
                {
                    DEBUG("Table is not an array");
                }
            }
        }
        else
        {
            DEBUG("No tables found in JSON data");
        }
    }

    void process(unsigned int sample_rate) override 
    {
        float in_voltage = inputs[IN]->getVoltage();
        in_voltage = clamp(in_voltage, 0.0f, 10.0f);

        float table_voltage = inputs[TABLE]->getVoltage();
        table_voltage = clamp(table_voltage, 0.0f, 10.0f);

        int table_index = static_cast<int>(table_voltage / 10.0f * tables.size());
        table_index = clamp(table_index, 0, static_cast<int>(tables.size()) - 1);

        std::vector<float>& current_table = tables[table_index];
        
        int lookup_index = static_cast<int>(in_voltage / 10.0f * current_table.size());
        lookup_index = clamp(lookup_index, 0, static_cast<int>(current_table.size()) - 1);
        
        outputs[OUTPUT]->setVoltage(current_table[lookup_index]);
    }

private:
    std::vector<std::vector<float>> tables;
};
