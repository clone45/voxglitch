// Rename to voxbuilder
#include <fstream>
#include "VoxbuilderLogger.hpp"

struct Inner : VoxglitchModule
{
    float audio_out = 0;
    float p1 = 0.0;
    float p2 = 0.0;
    float p3 = 0.0;
    float p4 = 0.0;
    float p5 = 0.0;
    float p6 = 0.0;
    float p7 = 0.0;
    float p8 = 0.0;

    float pitch = 0.0;
    float gate = 0.0;

    ModuleManager *module_manager;
    PatchRunner *patch_runner;
    IModule *terminal_output_module = nullptr;
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
        PARAM4_CV_INPUT,
        PARAM5_CV_INPUT,
        PARAM6_CV_INPUT,
        PARAM7_CV_INPUT,
        PARAM8_CV_INPUT,
        NUM_INPUTS
    };
    enum OutputIds
    {
        AUDIO_OUTPUT,
        NUM_OUTPUTS
    };

    Inner()
    {
        module_manager = new ModuleManager(&pitch, &gate, &p1, &p2, &p3, &p4, &p5, &p6, &p7, &p8);
        patch_runner = new PatchRunner();
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS);
    }

    void process(const ProcessArgs &args) override
    {
        float audio_out = 1.0;

        p1 = inputs[PARAM1_CV_INPUT].getVoltage();
        p2 = inputs[PARAM2_CV_INPUT].getVoltage();
        p3 = inputs[PARAM3_CV_INPUT].getVoltage();
        p4 = inputs[PARAM4_CV_INPUT].getVoltage();
        p5 = inputs[PARAM5_CV_INPUT].getVoltage();
        p6 = inputs[PARAM6_CV_INPUT].getVoltage();
        p7 = inputs[PARAM7_CV_INPUT].getVoltage();
        p8 = inputs[PARAM8_CV_INPUT].getVoltage();

        pitch = inputs[PITCH_INPUT].getVoltage();
        gate = inputs[GATE_INPUT].getVoltage();

        // TODO: Only update module_manager's sample rate when the rate changes

        // Calculate your audio output here
        if (module_manager->isReady())
        {
            // audio_out = module_manager->process(args.sampleRate);
            audio_out = patch_runner->process(args.sampleRate, terminal_output_module);
        }

        // Set the output value
        outputs[AUDIO_OUTPUT].setVoltage(audio_out);
    }

    void load_config_file(const std::string& filename)
    {
        // set module_manager ready to false
        module_manager->setReady(false);

        // Clean out any old data if the module_manager has been used before
        module_manager->clear();

        // clear the module config map
        std::vector<ModuleConfig*> modules;
        std::ifstream file(filename);

        json_t* root = json_load_file(filename.c_str(), 0, nullptr);
        json_t* modules_array = json_object_get(root, "modules");
        size_t modules_size = json_array_size(modules_array);

        for (size_t i = 0; i < modules_size; ++i)
        {
            json_t* module_obj = json_array_get(modules_array, i);

            std::string uuid = json_string_value(json_object_get(module_obj, "uuid"));
            std::string type = json_string_value(json_object_get(module_obj, "type"));

            // If I didn't use json_deep_copy below, problems would occur when
            // root is freed, which could cause intermittent crashes.

            //
            //  Load "defaults"
            // 
            json_t* defaults = nullptr;
            if(json_t* defaults_obj = json_object_get(module_obj, "defaults")) 
            {
                defaults = json_deep_copy(defaults_obj);
            }

            //
            //  Load "data"
            // 
            json_t* data = nullptr;
            if(json_t* data_obj = json_object_get(module_obj, "data")) 
            {
                data = json_deep_copy(data_obj);
            }

            modules.push_back(new ModuleConfig(uuid, type, defaults, data));
        }

        module_manager->setModuleConfigMap(modules);

        //
        // Load and set the connections
        //

        std::vector<Connection> connections;
        json_t* connections_array = json_object_get(root, "connections");
        size_t connections_size = json_array_size(connections_array);

        for (size_t i = 0; i < connections_size; ++i)
        {
            json_t* connection_obj = json_array_get(connections_array, i);

            json_t* src_obj = json_object_get(connection_obj, "src");
            std::string src_module_uuid = json_string_value(json_object_get(src_obj, "module_uuid"));
            unsigned int src_port_id = json_integer_value(json_object_get(src_obj, "port_id"));

            json_t* dst_obj = json_object_get(connection_obj, "dst");
            std::string dst_module_uuid = json_string_value(json_object_get(dst_obj, "module_uuid"));
            unsigned int dst_port_id = json_integer_value(json_object_get(dst_obj, "port_id"));

            // connections.emplace_back(src_module_id, src_port_id, dst_module_id, dst_port_id);
            connections.push_back(Connection(src_module_uuid, src_port_id, dst_module_uuid, dst_port_id));
        }

        module_manager->setConnections(connections);

        json_decref(root);

        // Tie the modules together and create the patch
        if (! module_manager->createPatch())
        {
            DEBUG("Failed to create patch");
        }
        else
        {
            terminal_output_module = module_manager->getTerminalOutputModule();
        }
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
