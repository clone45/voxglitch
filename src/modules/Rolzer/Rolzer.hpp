#pragma once

#include <cmath>
#include <vector>
#include <cstring>

struct Rolzer : Module
{
    static const int NUM_ROLLS = 5;
    static const int TOTAL_NODES = 24; // 3 + 4 + 5 + 6 + 6
    static const int NUM_GATE_OUTPUTS = 5;

    int rollSize[NUM_ROLLS] = {3, 4, 5, 6, 6};
    int rollNodeOffset[NUM_ROLLS] = {0, 3, 7, 12, 18};

    // Precomputed ring predecessor for each node
    int ringPredecessor[TOTAL_NODES] = {};

    enum ParamIds {
        ENUMS(TEMPO_PARAM, NUM_ROLLS),
        NUM_PARAMS
    };

    enum InputIds {
        NUM_INPUTS
    };

    enum OutputIds {
        ENUMS(GATE_OUTPUT, NUM_GATE_OUTPUTS),
        NUM_OUTPUTS
    };

    enum LightIds {
        NUM_LIGHTS
    };

    // Node DSP state
    float nodeState[TOTAL_NODES] = {};
    bool nodeFired[TOTAL_NODES] = {};
    float nodeSmoothed[TOTAL_NODES] = {};

    // Gate output routing — which node drives each gate output
    int gateSourceNode[NUM_GATE_OUTPUTS] = {0, 3, 7, 12, 18};

    // Gate pulse generators
    dsp::PulseGenerator gatePulse[NUM_GATE_OUTPUTS];

    // Internal patch connections (bidirectional)
    struct Connection {
        int nodeA;
        int nodeB;
    };
    std::vector<Connection> connections;

    // DSP tuning constants
    float decayRate = 2.0f;
    float ringImpulse = 0.5f;    // direct charge injection (fraction of threshold)
    float patchCoupling = 0.3f;  // direct charge injection from cross-patch fire
    float noiseAmount = 0.001f;  // per-sample jitter (not scaled by dt)

    static const int NUM_TEMPOS = 20;
    float tempoChargeRates[NUM_TEMPOS] = {
        0.25f, 0.5f, 0.9f, 1.5f, 2.5f,
        4.0f, 5.5f, 7.0f, 10.0f, 14.0f,
        20.0f, 28.0f, 38.0f, 48.0f, 60.0f,
        75.0f, 95.0f, 120.0f, 160.0f, 200.0f
    };

    Rolzer()
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        std::string rollNames[NUM_ROLLS] = {"Triangle", "Square", "Pentagon", "Hex A", "Hex B"};

        for (int r = 0; r < NUM_ROLLS; r++)
        {
            configParam(TEMPO_PARAM + r, 0.f, (float)(NUM_TEMPOS - 1), 7.f, rollNames[r] + " Tempo");
            getParamQuantity(TEMPO_PARAM + r)->snapEnabled = true;
        }

        for (int g = 0; g < NUM_GATE_OUTPUTS; g++)
            configOutput(GATE_OUTPUT + g, "Gate " + std::to_string(g + 1));

        // Precompute ring predecessors
        for (int r = 0; r < NUM_ROLLS; r++)
        {
            int offset = rollNodeOffset[r];
            int size = rollSize[r];
            for (int n = 0; n < size; n++)
            {
                int pred = (n == 0) ? (size - 1) : (n - 1);
                ringPredecessor[offset + n] = offset + pred;
            }
        }
    }

    int getRollForNode(int nodeIndex)
    {
        for (int r = NUM_ROLLS - 1; r >= 0; r--)
        {
            if (nodeIndex >= rollNodeOffset[r]) return r;
        }
        return 0;
    }

    float getChargeRate(int roll)
    {
        int idx = (int)params[TEMPO_PARAM + roll].getValue();
        idx = clamp(idx, 0, NUM_TEMPOS - 1);
        return tempoChargeRates[idx];
    }

    // Gate output routing

    void setGateSource(int gateIndex, int nodeIndex)
    {
        if (gateIndex < 0 || gateIndex >= NUM_GATE_OUTPUTS) return;
        if (nodeIndex < 0 || nodeIndex >= TOTAL_NODES) return;
        gateSourceNode[gateIndex] = nodeIndex;
    }

    // Returns the gate output index (0-4) if this node is assigned to one, or -1
    int getGateOutputForNode(int nodeIndex)
    {
        for (int g = 0; g < NUM_GATE_OUTPUTS; g++)
        {
            if (gateSourceNode[g] == nodeIndex) return g;
        }
        return -1;
    }

    // Connection management

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

    // DSP

    void process(const ProcessArgs &args) override
    {
        float dt = args.sampleTime;
        bool firedThisFrame[TOTAL_NODES] = {};

        for (int i = 0; i < TOTAL_NODES; i++)
        {
            int roll = getRollForNode(i);
            float chargeRate = getChargeRate(roll);

            // Direct impulse from ring predecessor firing
            if (nodeFired[ringPredecessor[i]])
                nodeState[i] += ringImpulse;

            // Direct impulse from cross-patch connections firing
            for (auto &conn : connections)
            {
                if (conn.nodeA == i && nodeFired[conn.nodeB])
                    nodeState[i] += patchCoupling;
                else if (conn.nodeB == i && nodeFired[conn.nodeA])
                    nodeState[i] += patchCoupling;
            }

            // Per-sample noise to prevent lock-up
            nodeState[i] += (random::uniform() - 0.5f) * noiseAmount;

            // Leaky integrator: continuous charge + natural decay
            nodeState[i] += (chargeRate - decayRate * nodeState[i]) * dt;
            nodeState[i] = std::max(0.0f, nodeState[i]);

            // Threshold and fire
            if (nodeState[i] >= 1.0f)
            {
                nodeState[i] = 0.0f;
                firedThisFrame[i] = true;
            }

            // Smooth node output for visualization (one-pole LP ~30Hz)
            float target = firedThisFrame[i] ? 1.0f : nodeState[i];
            nodeSmoothed[i] += (target - nodeSmoothed[i]) * 30.0f * dt;
        }

        std::memcpy(nodeFired, firedThisFrame, sizeof(nodeFired));

        // Gate outputs — fire when the assigned source node fires
        for (int g = 0; g < NUM_GATE_OUTPUTS; g++)
        {
            int srcNode = gateSourceNode[g];
            if (srcNode >= 0 && srcNode < TOTAL_NODES && firedThisFrame[srcNode])
                gatePulse[g].trigger(1e-3f);

            outputs[GATE_OUTPUT + g].setVoltage(gatePulse[g].process(dt) ? 10.0f : 0.0f);
        }
    }

    // Serialization

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
