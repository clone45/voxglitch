### File Format

Here are the requirements for the configuration file:

    The configuration file should specify what modules are being used in the patch, including multiple instances of a single type of module.
    The configuration file should specify what modules are connected to what other modules.
    An input can only have a single output connected to it, but a single output can be connected to multiple inputs.

Based on these requirements, it seems like the configuration file should define the following information for each module:

    The type of the module (e.g. "ADSR" or "Moog Filter").
    The unique identifier for the module.
    The inputs and outputs of the module, including their unique identifiers.
    The connections between the inputs and outputs.

