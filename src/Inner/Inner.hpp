// Rename to voxbuilder
#include <fstream>

struct Inner : VoxglitchModule
{
    float audio_out = 0;
    float p1 = 0.0;
    float p2 = 0.0;
    float p3 = 0.0;
    float pitch = 0.0;
    float gate = 0.0;

    ModuleManager *module_manager;
    
    std::string path = "";

    enum ParamIds
    {
        NUM_PARAMS
    };
    enum InputIds
    {
        PITCH_INPUT,
        GATE_INPUT,
        PARAM1_CV_INPUT,
        PARAM2_CV_INPUT,
        PARAM3_CV_INPUT,
        NUM_INPUTS
    };
    enum OutputIds
    {
        AUDIO_OUTPUT,
        NUM_OUTPUTS
    };

    Inner()
    {
        module_manager = new ModuleManager(&pitch, &gate, &p1, &p2, &p3);
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS);
    }

    void process(const ProcessArgs &args) override
    {
        float audio_out = 1.0;

        p1 = inputs[PARAM1_CV_INPUT].getVoltage();
        p2 = inputs[PARAM2_CV_INPUT].getVoltage();
        p3 = inputs[PARAM3_CV_INPUT].getVoltage();
        pitch = inputs[PITCH_INPUT].getVoltage();
        gate = inputs[GATE_INPUT].getVoltage();

        // Calculate your audio output here
        if (module_manager->isReady())
            audio_out = module_manager->process();

        // Set the output value
        outputs[AUDIO_OUTPUT].setVoltage(audio_out);
    }

    void load_config_file(const std::string &filename)
    {
        std::vector<ModuleConfig*> modules;
        std::ifstream file(filename);

        json_t *root = json_load_file(filename.c_str(), 0, nullptr);
        json_t *modules_array = json_object_get(root, "modules");
        size_t modules_size = json_array_size(modules_array);

        for (size_t i = 0; i < modules_size; ++i)
        {
            json_t *module_obj = json_array_get(modules_array, i);

            std::string name = json_string_value(json_object_get(module_obj, "name"));
            std::string type = json_string_value(json_object_get(module_obj, "type"));

            json_t *params_obj = json_object_get(module_obj, "params");
            std::map<std::string, float> params;

            if (params_obj)
            {
                const char *key;
                json_t *value;
                json_object_foreach(params_obj, key, value)
                {
                    params.emplace(key, (float)json_real_value(value));
                }
            }

            modules.push_back(new ModuleConfig(name, type, params));
        }

        module_manager->setModuleConfigMap(modules);

        //
        //  Load and set the connections
        //

        std::vector<std::pair<std::string, std::string>> connections;
        json_t* connections_array = json_object_get(root, "connections");
        size_t connections_size = json_array_size(connections_array);
        
        for (size_t i = 0; i < connections_size; ++i) 
        {
            json_t* connection_obj = json_array_get(connections_array, i);

            std::string output = json_string_value(json_object_get(connection_obj, "output"));
            std::string input = json_string_value(json_object_get(connection_obj, "input"));

            connections.emplace_back(output, input);
        }

        module_manager->setConnectionConfig(connections);

        json_decref(root);

        // Tie the modules together and create the patch
        module_manager->createPatch();
    }

    std::string selectFileVCV()
    {
        std::string filename_string = "";
        osdialog_filters *filters = osdialog_filters_parse("JSON:json");
        char *filename = osdialog_file(OSDIALOG_OPEN, "", NULL, filters);

        if (filename != NULL)
        {
            filename_string.assign(filename);
            osdialog_filters_free(filters);
            std::free(filename);
        }

        return (filename_string);
    }    
};
