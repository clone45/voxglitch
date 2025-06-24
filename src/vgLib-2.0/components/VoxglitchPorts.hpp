struct VoxglitchPort : SvgPort
{
    VoxglitchPort()
    {
        // this->shadow->opacity = 0;
    }
};

struct VoxglitchInputPort : VoxglitchPort
{
    VoxglitchInputPort()
    {
        setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/voxglitch_input_port.svg")));
    }
};

struct VoxglitchOutputPort : VoxglitchPort
{
    VoxglitchOutputPort()
    {
        setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/voxglitch_output_port.svg")));
    }
};

struct VoxglitchPolyPort : VoxglitchPort
{
    VoxglitchPolyPort()
    {
        setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/voxglitch_poly_port.svg")));
    }
};
