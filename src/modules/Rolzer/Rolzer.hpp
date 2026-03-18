#pragma once

#include <cmath>
#include <vector>
#include <cstring>

// Models a single transistor stage in the ring oscillator.
// Each node has a capacitor that charges exponentially toward a supply voltage,
// fires when it reaches threshold, and sends an inverted impulse to the next
// node — faithful to the Ciat-Lonbarde Rollz-5 / Rolzer circuit topology.
struct RollNode
{
    float capVoltage = 0.0f;      // Base capacitor voltage (charges toward threshold)
    float outputVoltage = 1.0f;   // Collector voltage (inverted — the sandrode output)
    float prevOutput = 1.0f;      // Previous sample's output (for AC coupling)
    bool fired = false;
    float smoothedOutput = 0.0f;

    void reset()
    {
        capVoltage = 0.0f;
        outputVoltage = 1.0f;
        prevOutput = 1.0f;
        fired = false;
        smoothedOutput = 0.0f;
    }

    void injectImpulse(float amount)
    {
        capVoltage += amount;
    }
};

// Models a ring oscillator with N inverting transistor stages.
// Even-count rings (4, 6) produce stable periodic oscillation because
// a pulse traverses the ring and arrives back in the same polarity.
// Odd-count rings (3, 5) produce chaotic, aperiodic behavior because
// a pulse arrives back inverted — the "paradox spiral."
struct Roll
{
    static const int MAX_NODES = 6;
    int size;
    RollNode nodes[MAX_NODES];

    // Circuit parameters matching hardware behavior
    static constexpr float SUPPLY_TARGET = 1.0f;
    static constexpr float THRESHOLD = 0.7f;
    static constexpr float RESET_VOLTAGE = 0.0f;
    static constexpr float NOISE_AMOUNT = 0.001f;

    // AC coupling through inter-stage coupling capacitor.
    // The coupling cap passes the CHANGE in the predecessor's collector
    // voltage to the current stage's base. This naturally produces:
    //   - Large impulse when predecessor fires (collector drops sharply)
    //   - Small continuous modulation during charging (collector recovers)
    // Replaces the old separate impulse + DC continuous coupling model.
    static constexpr float COUPLING_CAP = 0.5f;

    // Collector recovery speed relative to base charging rate.
    // Determines the timbre of the "swirl" — lower = darker/slower decay.
    static constexpr float COLLECTOR_SPEED = 2.0f;

    Roll() : size(4) {}
    Roll(int sz) : size(sz) {}

    RollNode& operator[](int i) { return nodes[i]; }
    const RollNode& operator[](int i) const { return nodes[i]; }

    // Process one sample of the ring oscillator.
    // chargeAlpha = 1 - exp(-dt/RC), precomputed from tempo setting.
    void process(float chargeAlpha, float dt)
    {
        // Snapshot collector voltages for order-independent AC coupling
        float snapOutput[MAX_NODES];
        float snapPrev[MAX_NODES];
        for (int n = 0; n < size; n++)
        {
            snapOutput[n] = nodes[n].outputVoltage;
            snapPrev[n] = nodes[n].prevOutput;
        }

        bool newFired[MAX_NODES] = {};

        for (int n = 0; n < size; n++)
        {
            RollNode &node = nodes[n];
            int pred = (n == 0) ? (size - 1) : (n - 1);

            // AC coupling through inter-stage coupling capacitor.
            // The cap passes the rate of change of the predecessor's
            // collector voltage. On fire events this is a large negative
            // delta (collector drops from ~1V to 0V), producing a sharp
            // inhibitory kick. During recovery it's a small positive
            // delta, gently pushing the next stage toward threshold.
            float predDelta = snapOutput[pred] - snapPrev[pred];
            node.capVoltage += predDelta * COUPLING_CAP;

            // Small noise to seed chaos and prevent digital lock-up
            node.capVoltage += (random::uniform() - 0.5f) * NOISE_AMOUNT;

            // Exponential RC charging toward supply target
            node.capVoltage += (SUPPLY_TARGET - node.capVoltage) * chargeAlpha;

            // Soft clamp: base-emitter junction limits negative excursion
            node.capVoltage = clamp(node.capVoltage, -0.3f, SUPPLY_TARGET);

            // Threshold detection: transistor turns on, cap discharges
            if (node.capVoltage >= THRESHOLD)
            {
                node.capVoltage = RESET_VOLTAGE;
                newFired[n] = true;
            }

            // Update collector (output) voltage — the sandrode signal.
            // Transistor ON (fired): collector drops sharply to ground (the "snap").
            // Transistor OFF: collector recovers exponentially toward supply
            // through the load resistor (the "swirl").
            if (newFired[n])
            {
                node.outputVoltage = 0.0f;
            }
            else
            {
                node.outputVoltage += (SUPPLY_TARGET - node.outputVoltage)
                    * chargeAlpha * COLLECTOR_SPEED;
            }

            // Smoothed output for visualization
            node.smoothedOutput += (node.outputVoltage - node.smoothedOutput) * 30.0f * dt;
        }

        // Update states for next frame
        for (int n = 0; n < size; n++)
        {
            nodes[n].fired = newFired[n];
            nodes[n].prevOutput = snapOutput[n];
        }
    }
};

struct Rolzer : Module
{
    static const int NUM_ROLLS = 5;
    static const int TOTAL_NODES = 24; // 3 + 4 + 5 + 6 + 6
    static const int NUM_GATE_OUTPUTS = 5;

    int rollSize[NUM_ROLLS] = {3, 4, 5, 6, 6};
    int rollNodeOffset[NUM_ROLLS] = {0, 3, 7, 12, 18};

    Roll rolls[NUM_ROLLS];

    enum ParamIds {
        TEMPO_PARAM,  // Single tempo selector (E6 cap series, shared by all rolls)
        NUM_PARAMS
    };

    enum InputIds {
        NUM_INPUTS
    };

    enum OutputIds {
        ENUMS(GATE_OUTPUT, NUM_GATE_OUTPUTS),
        ENUMS(WAVE_OUTPUT, NUM_ROLLS),
        NUM_OUTPUTS
    };

    enum LightIds {
        NUM_LIGHTS
    };

    // Gate output routing — which node drives each gate output
    int gateSourceNode[NUM_GATE_OUTPUTS] = {0, 3, 7, 12, 18};
    dsp::PulseGenerator gatePulse[NUM_GATE_OUTPUTS];

    // Internal patch connections (bidirectional sandrode coupling)
    struct Connection {
        int nodeA;
        int nodeB;
    };
    std::vector<Connection> connections;

    // Cross-patch coupling: brown jacks connected directly (no current limiting).
    // Impulse on fire + continuous voltage equalization between connected nodes.
    static constexpr float PATCH_IMPULSE = 0.3f;
    static constexpr float PATCH_CONTINUOUS = 200.0f;

    // E6 capacitor series RC time constants (seconds).
    // Matches the Rolzer hardware: each tempo module uses one fixed cap value.
    // RC = capacitance × ~66KΩ charge resistor (estimated from LTSpice data).
    // Position 0 = fastest (Presto), position 6 = slowest (Grave).
    static const int NUM_TEMPOS = 7;
    float rcTimeConstants[NUM_TEMPOS] = {
        0.066f,   // Presto   (1.0µF)
        0.099f,   // Allegro  (1.5µF)
        0.145f,   // Moderato (2.2µF)
        0.218f,   // Andante  (3.3µF)
        0.310f,   // Adagio   (4.7µF)
        0.449f,   // Lento    (6.8µF)
        0.660f    // Grave    (10.0µF)
    };

    Rolzer()
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        std::string rollNames[NUM_ROLLS] = {"Triangle", "Square", "Pentagon", "Hex A", "Hex B"};

        for (int r = 0; r < NUM_ROLLS; r++)
            rolls[r] = Roll(rollSize[r]);

        // Single tempo selector: matches hardware where each module has one
        // fixed capacitor value. Default to Andante (position 3).
        configParam(TEMPO_PARAM, 0.f, (float)(NUM_TEMPOS - 1), 3.f, "Tempo");
        getParamQuantity(TEMPO_PARAM)->snapEnabled = true;

        for (int g = 0; g < NUM_GATE_OUTPUTS; g++)
            configOutput(GATE_OUTPUT + g, "Gate " + std::to_string(g + 1));

        for (int r = 0; r < NUM_ROLLS; r++)
            configOutput(WAVE_OUTPUT + r, rollNames[r] + " Waveform");
    }

    // --- Node access ---

    RollNode& getNode(int globalIndex)
    {
        int r = getRollForNode(globalIndex);
        return rolls[r][globalIndex - rollNodeOffset[r]];
    }

    const RollNode& getNode(int globalIndex) const
    {
        int r = getRollForNode(globalIndex);
        return rolls[r][globalIndex - rollNodeOffset[r]];
    }

    int getRollForNode(int nodeIndex) const
    {
        for (int r = NUM_ROLLS - 1; r >= 0; r--)
        {
            if (nodeIndex >= rollNodeOffset[r]) return r;
        }
        return 0;
    }

    float getNodeSmoothedOutput(int globalIndex) const
    {
        return getNode(globalIndex).smoothedOutput;
    }

    // --- Tempo / RC ---

    float getChargeAlpha(float dt)
    {
        int idx = (int)params[TEMPO_PARAM].getValue();
        idx = clamp(idx, 0, NUM_TEMPOS - 1);
        return 1.0f - expf(-dt / rcTimeConstants[idx]);
    }

    // --- Gate output routing ---

    void setGateSource(int gateIndex, int nodeIndex)
    {
        if (gateIndex < 0 || gateIndex >= NUM_GATE_OUTPUTS) return;
        if (nodeIndex < 0 || nodeIndex >= TOTAL_NODES) return;
        gateSourceNode[gateIndex] = nodeIndex;
    }

    int getGateOutputForNode(int nodeIndex) const
    {
        for (int g = 0; g < NUM_GATE_OUTPUTS; g++)
        {
            if (gateSourceNode[g] == nodeIndex) return g;
        }
        return -1;
    }

    // --- Connection management ---

    void addConnection(int nodeA, int nodeB)
    {
        if (nodeA == nodeB) return;
        if (nodeA < 0 || nodeA >= TOTAL_NODES) return;
        if (nodeB < 0 || nodeB >= TOTAL_NODES) return;

        for (auto &c : connections)
        {
            if ((c.nodeA == nodeA && c.nodeB == nodeB) ||
                (c.nodeA == nodeB && c.nodeB == nodeA))
                return;
        }
        connections.push_back({nodeA, nodeB});
    }

    void removeConnection(int index)
    {
        if (index >= 0 && index < (int)connections.size())
            connections.erase(connections.begin() + index);
    }

    void removeConnectionsForNode(int nodeIndex)
    {
        connections.erase(
            std::remove_if(connections.begin(), connections.end(),
                [nodeIndex](Connection &c) {
                    return c.nodeA == nodeIndex || c.nodeB == nodeIndex;
                }),
            connections.end()
        );
    }

    // --- DSP ---

    void process(const ProcessArgs &args) override
    {
        float dt = args.sampleTime;

        // 1. Cross-patch coupling: sandrode (brown jack) connections.
        //    Brown jacks connect collectors directly (no current limiting),
        //    so connected nodes strongly influence each other.
        for (auto &conn : connections)
        {
            RollNode &nodeA = getNode(conn.nodeA);
            RollNode &nodeB = getNode(conn.nodeB);

            // Fire events inject impulse into connected node's base
            if (nodeA.fired)
                nodeB.injectImpulse(-PATCH_IMPULSE);
            if (nodeB.fired)
                nodeA.injectImpulse(-PATCH_IMPULSE);

            // Collector voltage equalization: directly wired outputs
            // pull toward each other through shared junction
            float delta = nodeA.outputVoltage - nodeB.outputVoltage;
            float coupling = PATCH_CONTINUOUS * delta * dt;
            nodeA.outputVoltage -= coupling;
            nodeB.outputVoltage += coupling;
        }

        // 2. Process each roll's internal ring oscillator.
        //    All rolls share the same RC time constant (one cap value per module).
        float chargeAlpha = getChargeAlpha(dt);
        for (int r = 0; r < NUM_ROLLS; r++)
            rolls[r].process(chargeAlpha, dt);

        // 3. Gate outputs — fire a 1ms pulse when the assigned source node fires
        for (int g = 0; g < NUM_GATE_OUTPUTS; g++)
        {
            int srcNode = gateSourceNode[g];
            if (srcNode >= 0 && srcNode < TOTAL_NODES && getNode(srcNode).fired)
                gatePulse[g].trigger(1e-3f);

            outputs[GATE_OUTPUT + g].setVoltage(gatePulse[g].process(dt) ? 10.0f : 0.0f);
        }

        // 4. Waveform outputs — collector (sandrode) voltage scaled to ±5V.
        //    Sharp drop on fire (the "snap"), smooth exponential recovery
        //    (the "swirl"), modulated by inter-node coupling.
        for (int r = 0; r < NUM_ROLLS; r++)
        {
            int srcNode = gateSourceNode[r];
            if (srcNode >= 0 && srcNode < TOTAL_NODES)
            {
                float v = getNode(srcNode).outputVoltage;
                float scaled = v * 10.0f - 5.0f;
                outputs[WAVE_OUTPUT + r].setVoltage(clamp(scaled, -10.0f, 10.0f));
            }
        }
    }

    // --- Serialization ---

    json_t *dataToJson() override
    {
        json_t *rootJ = json_object();

        json_t *connsJ = json_array();
        for (auto &c : connections)
        {
            json_t *connJ = json_object();
            json_object_set_new(connJ, "a", json_integer(c.nodeA));
            json_object_set_new(connJ, "b", json_integer(c.nodeB));
            json_array_append_new(connsJ, connJ);
        }
        json_object_set_new(rootJ, "connections", connsJ);

        json_t *gatesJ = json_array();
        for (int g = 0; g < NUM_GATE_OUTPUTS; g++)
            json_array_append_new(gatesJ, json_integer(gateSourceNode[g]));
        json_object_set_new(rootJ, "gateSources", gatesJ);

        return rootJ;
    }

    void dataFromJson(json_t *rootJ) override
    {
        connections.clear();

        json_t *connsJ = json_object_get(rootJ, "connections");
        if (connsJ)
        {
            size_t index;
            json_t *connJ;
            json_array_foreach(connsJ, index, connJ)
            {
                int a = json_integer_value(json_object_get(connJ, "a"));
                int b = json_integer_value(json_object_get(connJ, "b"));
                addConnection(a, b);
            }
        }

        json_t *gatesJ = json_object_get(rootJ, "gateSources");
        if (gatesJ)
        {
            for (int g = 0; g < NUM_GATE_OUTPUTS; g++)
            {
                json_t *val = json_array_get(gatesJ, g);
                if (val)
                    gateSourceNode[g] = json_integer_value(val);
            }
        }
    }
};
