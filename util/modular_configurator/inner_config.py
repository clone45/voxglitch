import json
import modules

def main_menu():
    print("\nSelect an option:")
    print("1. Add a module")
    print("2. Add a connection")
    print("3. Display patch")
    print("4. Save configuration file")
    print("5. Quit")

def add_module(modules_list, module_type):
    name = f"{module_type.lower()}_{len(modules_list)}"
    modules_list.append({"name": name, "type": module_type, "params": {}})
    return name

def add_connection(connections_list, output_module, output_port, input_module, input_port):
    connections_list.append({
        "output": f"{output_module}.{output_port}",
        "input": f"{input_module}.{input_port}"
    })

def save_configuration_file(filename, modules_list, connections_list):
    config = {
        "modules": modules_list,
        "connections": connections_list
    }
    with open(filename, 'w') as f:
        json.dump(config, f, indent=4)

def choice_add_module(modules_list):
    print("\nAvailable module types:")
    for idx, mod in enumerate(modules.modules):
        print(f"{idx + 1}. {mod['type']}")
    module_choice = int(input("Select a module type: ")) - 1
    module_type = modules.modules[module_choice]['type']
    name = add_module(modules_list, module_type)
    print(f"Added {module_type} module with name {name}")

def choice_add_connection(modules_list, connections_list):
    print("\nAvailable modules:")
    for idx, mod in enumerate(modules_list):
        print(f"{idx + 1}. {mod['name']}")
    output_module_idx = int(input("Select an output module: ")) - 1
    output_module = modules_list[output_module_idx]['name']
    output_module_type = modules_list[output_module_idx]['type']
    output_ports = [mod for mod in modules.modules if mod["type"] == output_module_type][0]["outputs"]

    print("\nAvailable output ports:")
    for idx, port in enumerate(output_ports):
        print(f"{idx + 1}. {port}")
    output_port_idx = int(input("Select an output port: ")) - 1
    output_port = output_ports[output_port_idx]

    input_module_idx = int(input("Select an input module: ")) - 1
    input_module = modules_list[input_module_idx]['name']
    input_module_type = modules_list[input_module_idx]['type']
    input_ports = [mod for mod in modules.modules if mod["type"] == input_module_type][0]["inputs"]

    print("\nAvailable input ports:")
    for idx, port in enumerate(input_ports):
        print(f"{idx + 1}. {port}")
    input_port_idx = int(input("Select an input port: ")) - 1
    input_port = input_ports[input_port_idx]

    add_connection(connections_list, output_module, output_port, input_module, input_port)
    print(f"Added connection from {output_module}.{output_port} to {input_module}.{input_port}")

def choice_visualize_patch(modules_list, connections_list):
    print("\nModules:")
    for module in modules_list:
        print(f"{module['name']} ({module['type']})")

    print("\nConnections:")
    for connection in connections_list:
        print(f"{connection['output']} -> {connection['input']}")


def choice_save_configuration_file(modules_list, connections_list):
    filename = input("Enter the filename for the configuration file (e.g., config.json): ")
    save_configuration_file(filename, modules_list, connections_list)
    print(f"Configuration file saved as {filename}")


def main():
    modules_list = []
    connections_list = []
    while True:
        main_menu()
        choice = input("Enter your choice: ")
        if choice == "1":
            choice_add_module(modules_list)
        elif choice == "2":
            choice_add_connection(modules_list, connections_list)
        elif choice == "3":
            choice_save_configuration_file(modules_list, connections_list)
        elif choice == "4":
            choice_visualize_patch(modules_list, connections_list)         
        elif choice == "5":
            break
        else:
            print("Invalid choice. Please try again.")

if __name__ == "__main__":
    main()
