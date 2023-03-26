#include <fstream>

using json = nlohmann::json;

void loadModulesFromConfig(const std::string& config_file_path) {
    // Load the configuration file
    std::ifstream input_file(config_file_path);
    json config;
    input_file >> config;

    // Iterate over the module configs and create the modules
    for (const auto& module_config_data : config["modules"]) {
        std::string module_name = module_config_data["name"].get<std::string>();
        std::string module_type = module_config_data["type"].get<std::string>();
        auto module = createModule(module_type);
        module->setName(module_name);
        addModule(module);

        // Set the module parameters
        for (const auto& [param_name, param_value] : module_config_data["params"].items()) {
            module->setParameter(param_name, param_value.get<float>());
        }
    }

    // Create the patch
    createPatch();

    // Load the input and output ports from the config file
    loadPortsFromConfig(config);
}

void ModuleManager::loadPortsFromConfig(const json& config) {
    for (const auto& module_config_data : config["modules"]) {
        std::string module_name = module_config_data["name"].get<std::string>();
        auto module = getModuleByName(module_name);
        if (!module) {
            // handle error
            continue;
        }

        // iterate over the input ports and add them to the module's input port map
        if (module_config_data.contains("inputs")) {
            for (const auto& input_config : module_config_data["inputs"]) {
                std::string input_name = input_config["name"].get<std::string>();
                auto input = std::make_shared<Input>(module);
                module->addInput(input);
                input_ports[module_name + "." + input_name] = input;
            }
        }

        // iterate over the output ports and add them to the module's output port map
        if (module_config_data.contains("outputs")) {
            for (const auto& output_config : module_config_data["outputs"]) {
                std::string output_name = output_config["name"].get<std::string>();
                auto output = std::make_shared<Output>(module);
                module->addOutput(output);
                output_ports[module_name + "." + output_name] = output;
            }
        }
    }
}