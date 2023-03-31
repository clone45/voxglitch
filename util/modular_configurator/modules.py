modules = [
    {
        "type": "ADSR", 
        "inputs": ["TRIGGER_INPUT_PORT", "GATE_INPUT_PORT", "ATTACK_INPUT_PORT", "DECAY_INPUT_PORT", "SUSTAIN_INPUT_PORT", "RELEASE_INPUT_PORT"],
        "outputs": ["OUTPUT_PORT"]
    },
    {
        "type": "EXPONENTIAL_VCA",
        "inputs": ["INPUT_PORT", "GAIN_INPUT_PORT"],
        "outputs": ["OUTPUT_PORT"]
    },
    {
        "type": "GATE_INPUT",
        "inputs": [],
        "outputs": ["OUTPUT_PORT"]
    },
    {
        "type": "LFO",
        "inputs": ["FREQUENCY_INPUT_PORT", "OUTPUT_PORT"],
        "outputs": ["OUTPUT_PORT"]
    },
    {
        "type": "LINEAR_VCA",
        "inputs": ["INPUT_PORT", "GAIN_INPUT_PORT"],
        "outputs": ["OUTPUT_PORT"]
    },
    {
        "type": "LOWPASS_FILTER",
        "inputs": ["INPUT_PORT", "CUTOFF_INPUT_PORT"],
        "outputs": ["OUTPUT_PORT"]
    },
    {
        "type": "MORPHING_FILTER",
        "inputs": ["INPUT_PORT", "CUTOFF_INPUT_PORT", "RESONANCE_INPUT_PORT", "MORPH_INPUT_PORT"],
        "outputs": ["OUTPUT_PORT"]
    }       
    {
        "type": "PARAM1",
        "inputs": [],
        "outputs": ["OUTPUT_PORT"]
    },
    {
        "type": "PARAM2",
        "inputs": [],
        "outputs": ["OUTPUT_PORT"]
    },
    {
        "type": "PARAM3",
        "inputs": [],
        "outputs": ["OUTPUT_PORT"]
    },
    {
        "type": "PARAM4",
        "inputs": [],
        "outputs": ["OUTPUT_PORT"]
    },
    {
        "type": "PARAM5",
        "inputs": [],
        "outputs": ["OUTPUT_PORT"]
    },
    {
        "type": "PARAM6",
        "inputs": [],
        "outputs": ["OUTPUT_PORT"]
    },
    {
        "type": "PARAM7",
        "inputs": [],
        "outputs": ["OUTPUT_PORT"]
    },
    {
        "type": "PARAM8",
        "inputs": [],
        "outputs": ["OUTPUT_PORT"]
    },
    {
        "type": "OUTPUT",
        "inputs": ["INPUT_PORT"],
        "outputs": []
    },
    {
        "type": "SCHROEDER_REVERB",
        "inputs": ["INPUT_PORT", "MIX_INPUT_PORT", "DECAY_INPUT_PORT"],
        "outputs": ["OUTPUT_PORT"]
    }, 
    {
        "type": "TB303_FILTER",
        "inputs": ["INPUT_PORT", "CUTOFF_INPUT_PORT", "RESONANCE_INPUT_PORT"],
        "outputs": ["OUTPUT_PORT"]
    },     
    {
        "type": "TB303_OSCILLATOR",
        "inputs": ["FREQUENCY_INPUT_PORT", "RESONANCE_INPUT_PORT"],
        "outputs": ["OUTPUT_PORT"]
    },     
    {
        "type": "VCO",
        "inputs": ["FREQUENCY_INPUT_PORT", "WAVEFORM_INPUT_PORT"],
        "outputs": ["OUTPUT_PORT"]
    },           
    {
        "type": "WAVETABLE_OSCILLATOR", 
        "inputs": ["FREQUENCY_INPUT_PORT", "WAVEFORM_INPUT_PORT", "OUTPUT_PORT"],
        "outputs": ["OUTPUT_PORT"]
    }
]