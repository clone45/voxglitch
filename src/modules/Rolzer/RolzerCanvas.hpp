#pragma once

struct RolzerCanvas : TransparentWidget
{
    Rolzer *module = nullptr;

    // Layout
    Vec rollCenter[Rolzer::NUM_ROLLS];
    float rollRadius[Rolzer::NUM_ROLLS];
    Vec nodePos[Rolzer::TOTAL_NODES];
    float nodeCircleRadius = 4.5f;
    bool layoutComputed = false;

    // Roll labels
    std::string rollLabels[Rolzer::NUM_ROLLS] = {"TRI", "SQR", "PENT", "HEX A", "HEX B"};

    // Gate output colors (for assignment badges)
    NVGcolor gateColors[Rolzer::NUM_GATE_OUTPUTS] = {
        nvgRGB(230, 80, 80),    // 1: red
        nvgRGB(80, 160, 230),   // 2: blue
        nvgRGB(80, 200, 100),   // 3: green
        nvgRGB(230, 160, 50),   // 4: orange
        nvgRGB(180, 100, 220)   // 5: purple
    };

    // Interaction state
    enum DragMode {
        DRAG_NONE,
        DRAG_CABLE
    };

    DragMode dragMode = DRAG_NONE;
    int cableStartNode = -1;
    Vec cableEndPos;

    // Hover state
    int hoverNode = -1;

    // Colors
    NVGcolor bgColor = nvgRGB(30, 30, 32);
    NVGcolor polygonColor = nvgRGBA(80, 80, 90, 140);
    NVGcolor nodeIdleColor = nvgRGB(50, 50, 55);
    NVGcolor nodeOutlineColor = nvgRGB(130, 130, 140);
    NVGcolor nodeHoverOutlineColor = nvgRGB(220, 200, 100);
    NVGcolor cableColor = nvgRGBA(220, 170, 40, 220);
    NVGcolor dragCableColor = nvgRGBA(220, 170, 40, 140);
    NVGcolor labelColor = nvgRGBA(140, 140, 150, 180);

    float cableThickness = 2.0f;

    void computeLayout()
    {
        float w = box.size.x;
        float h = box.size.y;
        float baseRadius = std::min(w / 3.0f, h / 2.0f) * 0.22f;

        // Row 1: Triangle, Square, Pentagon
        rollCenter[0] = Vec(w * 0.17f, h * 0.33f);
        rollCenter[1] = Vec(w * 0.50f, h * 0.33f);
        rollCenter[2] = Vec(w * 0.83f, h * 0.33f);
        // Row 2: Hex A, Hex B
        rollCenter[3] = Vec(w * 0.33f, h * 0.73f);
        rollCenter[4] = Vec(w * 0.67f, h * 0.73f);

        for (int r = 0; r < Rolzer::NUM_ROLLS; r++)
        {
            float sizeFactor = module ? (float)module->rollSize[r] / 5.0f + 0.5f : 1.0f;
            rollRadius[r] = baseRadius * sizeFactor;
        }

        if (module)
        {
            for (int r = 0; r < Rolzer::NUM_ROLLS; r++)
            {
                int offset = module->rollNodeOffset[r];
                int size = module->rollSize[r];
                for (int n = 0; n < size; n++)
                {
                    float angle = 2.0f * (float)M_PI * n / size - (float)M_PI / 2.0f;
                    nodePos[offset + n] = Vec(
                        rollCenter[r].x + cosf(angle) * rollRadius[r],
                        rollCenter[r].y + sinf(angle) * rollRadius[r]);
                }
            }
        }

        layoutComputed = true;
    }

    int getNodeAtPos(Vec pos)
    {
        if (!module) return -1;
        float hitRadius = nodeCircleRadius + 4.0f;
        for (int i = 0; i < Rolzer::TOTAL_NODES; i++)
        {
            Vec delta = pos.minus(nodePos[i]);
            if (delta.x * delta.x + delta.y * delta.y < hitRadius * hitRadius)
                return i;
        }
        return -1;
    }

    // ========================================================================
    // Drawing — static content
    // ========================================================================

    void draw(const DrawArgs &args) override
    {
        if (!layoutComputed) computeLayout();

        NVGcontext *vg = args.vg;

        // Background
        nvgBeginPath(vg);
        nvgRect(vg, 0, 0, box.size.x, box.size.y);
        nvgFillColor(vg, bgColor);
        nvgFill(vg);

        if (!module)
        {
            nvgFontSize(vg, 14.0f);
            nvgFillColor(vg, labelColor);
            nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
            nvgText(vg, box.size.x * 0.5f, box.size.y * 0.5f, "ROLZER", nullptr);
            TransparentWidget::draw(args);
            return;
        }

        // Draw polygon outlines
        for (int r = 0; r < Rolzer::NUM_ROLLS; r++)
            drawRollPolygon(vg, r);

        // Draw roll labels
        for (int r = 0; r < Rolzer::NUM_ROLLS; r++)
            drawRollLabel(vg, r);

        TransparentWidget::draw(args);
    }

    // ========================================================================
    // Drawing — dynamic content (cables, nodes, glow)
    // ========================================================================

    void drawLayer(const DrawArgs &args, int layer) override
    {
        if (layer == 1 && module)
        {
            if (!layoutComputed) computeLayout();

            NVGcontext *vg = args.vg;

            // Draw user connections (cables)
            for (auto &conn : module->connections)
                drawCableCurve(vg, nodePos[conn.nodeA], nodePos[conn.nodeB], cableColor, 1.0f);

            // Draw active drag cable
            if (dragMode == DRAG_CABLE && cableStartNode >= 0)
                drawCableCurve(vg, nodePos[cableStartNode], cableEndPos, dragCableColor, 0.7f);

            // Draw nodes (on top of cables)
            for (int i = 0; i < Rolzer::TOTAL_NODES; i++)
                drawNode(vg, i);
        }

        TransparentWidget::drawLayer(args, layer);
    }

    void drawRollPolygon(NVGcontext *vg, int r)
    {
        int offset = module->rollNodeOffset[r];
        int size = module->rollSize[r];

        nvgBeginPath(vg);
        for (int n = 0; n <= size; n++)
        {
            int idx = offset + (n % size);
            if (n == 0)
                nvgMoveTo(vg, nodePos[idx].x, nodePos[idx].y);
            else
                nvgLineTo(vg, nodePos[idx].x, nodePos[idx].y);
        }
        nvgClosePath(vg);
        nvgStrokeColor(vg, polygonColor);
        nvgStrokeWidth(vg, 1.5f);
        nvgStroke(vg);
    }

    void drawRollLabel(NVGcontext *vg, int r)
    {
        nvgFontSize(vg, 10.0f);
        nvgFillColor(vg, labelColor);
        nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
        nvgText(vg, rollCenter[r].x, rollCenter[r].y, rollLabels[r].c_str(), nullptr);
    }

    void drawNode(NVGcontext *vg, int nodeIndex)
    {
        Vec pos = nodePos[nodeIndex];
        float activity = module ? module->nodeSmoothed[nodeIndex] : 0.0f;
        bool isHovered = (nodeIndex == hoverNode);
        int gateOutput = module ? module->getGateOutputForNode(nodeIndex) : -1;

        // Filled circle
        nvgBeginPath(vg);
        nvgCircle(vg, pos.x, pos.y, nodeCircleRadius);
        nvgFillColor(vg, nodeIdleColor);
        nvgFill(vg);

        // Activity glow
        if (activity > 0.01f)
        {
            nvgBeginPath(vg);
            nvgCircle(vg, pos.x, pos.y, nodeCircleRadius - 0.5f);
            unsigned char a = (unsigned char)(clamp(activity, 0.f, 1.f) * 255);
            nvgFillColor(vg, nvgRGBA(255, 200, 50, a));
            nvgFill(vg);
        }

        // Outline — color-coded if assigned to a gate output, brighter on hover
        nvgBeginPath(vg);
        nvgCircle(vg, pos.x, pos.y, nodeCircleRadius);
        if (gateOutput >= 0)
        {
            nvgStrokeColor(vg, gateColors[gateOutput]);
            nvgStrokeWidth(vg, 2.0f);
        }
        else if (isHovered)
        {
            nvgStrokeColor(vg, nodeHoverOutlineColor);
            nvgStrokeWidth(vg, 2.0f);
        }
        else
        {
            nvgStrokeColor(vg, nodeOutlineColor);
            nvgStrokeWidth(vg, 1.0f);
        }
        nvgStroke(vg);

        // Gate output badge — small number below the node
        if (gateOutput >= 0)
        {
            char badge[4];
            snprintf(badge, sizeof(badge), "%d", gateOutput + 1);
            nvgFontSize(vg, 8.0f);
            nvgFillColor(vg, gateColors[gateOutput]);
            nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_TOP);
            nvgText(vg, pos.x, pos.y + nodeCircleRadius + 1.5f, badge, nullptr);
        }
    }

    void drawCableCurve(NVGcontext *vg, Vec start, Vec end, NVGcolor color, float alpha)
    {
        float dx = end.x - start.x;
        float dy = end.y - start.y;
        float dist = std::sqrt(dx * dx + dy * dy);

        float droop = std::min(dist * 0.15f, 20.0f);
        Vec ctrl1 = Vec(start.x + dx * 0.3f, start.y + dy * 0.3f + droop);
        Vec ctrl2 = Vec(end.x - dx * 0.3f, end.y - dy * 0.3f + droop);

        // Cable shadow
        nvgBeginPath(vg);
        nvgMoveTo(vg, start.x + 1, start.y + 1);
        nvgBezierTo(vg, ctrl1.x + 1, ctrl1.y + 1, ctrl2.x + 1, ctrl2.y + 1, end.x + 1, end.y + 1);
        nvgStrokeColor(vg, nvgRGBA(0, 0, 0, (int)(50 * alpha)));
        nvgStrokeWidth(vg, cableThickness + 1.5f);
        nvgLineCap(vg, NVG_ROUND);
        nvgStroke(vg);

        // Cable body
        nvgBeginPath(vg);
        nvgMoveTo(vg, start.x, start.y);
        nvgBezierTo(vg, ctrl1.x, ctrl1.y, ctrl2.x, ctrl2.y, end.x, end.y);
        NVGcolor bodyColor = color;
        bodyColor.a = alpha;
        nvgStrokeColor(vg, bodyColor);
        nvgStrokeWidth(vg, cableThickness);
        nvgLineCap(vg, NVG_ROUND);
        nvgStroke(vg);
    }

    // ========================================================================
    // Interaction
    // ========================================================================

    void onButton(const ButtonEvent &e) override
    {
        if (!module) return;

        if (e.button == GLFW_MOUSE_BUTTON_LEFT)
        {
            if (e.action == GLFW_PRESS)
            {
                int node = getNodeAtPos(e.pos);
                if (node >= 0)
                {
                    dragMode = DRAG_CABLE;
                    cableStartNode = node;
                    cableEndPos = e.pos;
                    e.consume(this);
                }
            }
            else if (e.action == GLFW_RELEASE)
            {
                if (dragMode == DRAG_CABLE && cableStartNode >= 0)
                {
                    int target = getNodeAtPos(cableEndPos);
                    if (target >= 0 && target != cableStartNode)
                        module->addConnection(cableStartNode, target);
                }

                dragMode = DRAG_NONE;
                cableStartNode = -1;
                e.consume(this);
            }
        }

        if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_RIGHT)
        {
            int node = getNodeAtPos(e.pos);
            if (node >= 0)
            {
                createNodeContextMenu(node);
                e.consume(this);
            }
        }
    }

    void onDragMove(const DragMoveEvent &e) override
    {
        TransparentWidget::onDragMove(e);

        if (dragMode == DRAG_CABLE)
        {
            float zoom = getAbsoluteZoom();
            Vec delta = e.mouseDelta.div(zoom);
            cableEndPos = cableEndPos.plus(delta);
        }
    }

    void onDragEnd(const DragEndEvent &e) override
    {
        TransparentWidget::onDragEnd(e);
        dragMode = DRAG_NONE;
        cableStartNode = -1;
    }

    void onHover(const HoverEvent &e) override
    {
        TransparentWidget::onHover(e);
        hoverNode = getNodeAtPos(e.pos);
        e.consume(this);
    }

    void onLeave(const LeaveEvent &e) override
    {
        TransparentWidget::onLeave(e);
        hoverNode = -1;
    }

    // ========================================================================
    // Context menu
    // ========================================================================

    void createNodeContextMenu(int nodeIndex)
    {
        if (!module) return;

        Menu *menu = createMenu();

        int roll = module->getRollForNode(nodeIndex);
        int localNode = nodeIndex - module->rollNodeOffset[roll];
        menu->addChild(createMenuLabel(rollLabels[roll] + " Node " + std::to_string(localNode + 1)));

        // Gate output routing
        menu->addChild(new MenuSeparator);
        menu->addChild(createMenuLabel("Route to Gate Output"));

        for (int g = 0; g < Rolzer::NUM_GATE_OUTPUTS; g++)
        {
            bool isAssigned = (module->gateSourceNode[g] == nodeIndex);
            std::string label = "Gate " + std::to_string(g + 1);
            int gateIdx = g;
            menu->addChild(createMenuItem(label, isAssigned ? "***" : "",
                [=]() { module->setGateSource(gateIdx, nodeIndex); }));
        }

        // Disconnect cables
        menu->addChild(new MenuSeparator);

        bool hasConnections = false;
        for (int i = (int)module->connections.size() - 1; i >= 0; i--)
        {
            auto &conn = module->connections[i];
            if (conn.nodeA == nodeIndex || conn.nodeB == nodeIndex)
            {
                hasConnections = true;
                int other = (conn.nodeA == nodeIndex) ? conn.nodeB : conn.nodeA;
                int otherRoll = module->getRollForNode(other);
                int otherLocal = other - module->rollNodeOffset[otherRoll];
                std::string label = "Disconnect " + rollLabels[otherRoll] +
                    " Node " + std::to_string(otherLocal + 1);
                int idx = i;
                menu->addChild(createMenuItem(label, "",
                    [=]() { module->removeConnection(idx); }));
            }
        }

        if (!hasConnections)
            menu->addChild(createMenuLabel("No connections"));
    }
};
